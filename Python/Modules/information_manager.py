import math
import typing
from enum import Enum
from typing import Optional, List

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2
from sc2.unit import UnitOrder

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

STEPS_PER_SECOND = 22.4

class WorkerRole(Enum):
    IDLE = 0
    MINERALS = 1
    GAS = 2
    BUILD = 3

class TownhallData:
    current_harvesters: int
    position: Point2
    tag: int

    def __init__(self, position: Point2, tag: int) -> None:
        self.current_harvesters = 0
        self.position = position
        self.tag = tag

class GasBuildingData:
    current_harvesters: int
    position: Point2
    tag: int

    def __init__(self, position: Point2, tag: int) -> None:
        self.current_harvesters = 0
        self.position = position
        self.tag = tag

class BarracksData:
    position: Point2
    tag: int

    def __init__(self, position: Point2, tag: int) -> None:
        self.position = position
        self.tag = tag

class SupplyDepotData:
    position: Point2
    tag: int

    def __init__(self, position: Point2, tag: int) -> None:
        self.position = position
        self.tag = tag

class MarineData:
    tag: int

    def __init__(self, tag: int) -> None:
        self.tag = tag

class WorkerData:
    assigned_to_tag: Optional[int]
    role: WorkerRole
    tag: int

    def __init__(self, role: WorkerRole, tag: int) -> None:
        self.role = role
        self.assigned_to_tag = None
        self.tag = tag

    def assign_to(self, assigned_to_tag: Optional[int], role: WorkerRole) -> None:
        self.assigned_to_tag = assigned_to_tag
        self.role = role


class InformationManager:
    worker_data: dict[int, WorkerData]
    th_data: dict[int, TownhallData]
    gas_data: dict[int, GasBuildingData]
    barracks_data: dict[int, BarracksData]
    supply_depot_data: dict[int, SupplyDepotData]
    marine_data: dict[int, MarineData]
    build_times: dict[UnitTypeId, int]
    el_list: dict[Point2, bool]
    completed_bases = set()

    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.worker_data = {}
        self.th_data = {}
        self.gas_data = {}
        self.barracks_data = {}
        self.supply_depot_data = {}
        self.marine_data = {}
        self.el_list = {}
        for el in self.bot.expansion_locations_list:
            self.el_list.update({el: False})
            if el.distance_to(self.bot.start_location) < 15:
                self.el_list[el] = True
        for worker in self.bot.workers:
            self.worker_data.update({worker.tag: WorkerData(WorkerRole.IDLE, worker.tag)})
        for th in self.bot.townhalls:
            self.th_data.update({th.tag: TownhallData(th.position, th.tag)})
        for geyser in self.bot.gas_buildings:
            self.gas_data.update({geyser.tag: GasBuildingData(geyser.position, geyser.tag)})
        for barracks in self.bot.structures.filter(lambda s: s.type_id == UnitTypeId.BARRACKS):
            self.barracks_data.update({barracks.tag: BarracksData(barracks.position, barracks.tag)})
        for depot in self.bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT):
            self.supply_depot_data.update({depot.tag: SupplyDepotData(depot.position, depot.tag)})
        for marine in self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE):
            self.marine_data.update({marine.tag: MarineData(marine.tag)})
        self.build_times = {
            UnitTypeId.COMMANDCENTER: math.ceil(self.bot.game_data.units[UnitTypeId.COMMANDCENTER.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.REFINERY: math.ceil(self.bot.game_data.units[UnitTypeId.REFINERY.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.SCV: math.ceil(self.bot.game_data.units[UnitTypeId.SCV.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.SUPPLYDEPOT: math.ceil(self.bot.game_data.units[UnitTypeId.SUPPLYDEPOT.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.BARRACKS: math.ceil(self.bot.game_data.units[UnitTypeId.BARRACKS.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.MARINE: math.ceil(self.bot.game_data.units[UnitTypeId.MARINE.value]._proto.build_time / STEPS_PER_SECOND),
        }

    async def remove_unit_by_tag(self, tag: int) -> None:
        if tag in self.worker_data:
            # remove assigned worker from assigned structure
            if self.worker_data[tag].assigned_to_tag is not None:
                if self.worker_data[tag].role == WorkerRole.MINERALS:
                    self.th_data[self.worker_data[tag].assigned_to_tag].current_harvesters -= 1
                elif self.worker_data[tag].role == WorkerRole.GAS:
                    self.gas_data[self.worker_data[tag].assigned_to_tag].current_harvesters -= 1
            if self.bot.base_worker and self.bot.base_worker.tag == tag:
                self.el_list[self.bot.new_base_location] = False
                self.bot.base_worker = None
                self.bot.new_base_location = None
            self.worker_data.pop(tag)
        elif tag in self.th_data:
            # remove all assigned workers from the townhall, make location available
            for worker in self.worker_data.values():
                if worker.assigned_to_tag == tag:
                    worker.assigned_to_tag = None
                    worker.role = WorkerRole.IDLE
            self.el_list[self.th_data[tag].position] = False
            if tag in self.completed_bases:
                self.completed_bases.remove(tag)
            if tag in self.th_data:
                self.th_data.pop(tag)
        elif tag in self.gas_data:
            # remove all assigned workers from the gas geyser, make location available
            for worker in self.worker_data.values():
                if worker.assigned_to_tag == tag:
                    worker.assigned_to_tag = None
                    worker.role = WorkerRole.IDLE
            # the tag of the closest townhall
            position = self.gas_data[tag].position
            closest_th_tag = min(self.th_data, key=lambda th: position.distance_to(self.th_data[th].position))
            if position.distance_to(self.th_data[closest_th_tag].position) < 15:
                self.completed_bases.remove(closest_th_tag)
            self.gas_data.pop(tag)
        elif tag in self.barracks_data:
            self.barracks_data.pop(tag)
        elif tag in self.supply_depot_data:
            self.supply_depot_data.pop(tag)
        elif tag in self.marine_data:
            self.marine_data.pop(tag)