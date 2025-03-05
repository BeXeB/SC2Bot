import math
import typing
from enum import Enum
from typing import Optional, List

from sc2.ids.ability_id import AbilityId
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
    townhall_data: dict[int, TownhallData]
    gas_data: dict[int, GasBuildingData]
    barracks_data: dict[int, BarracksData]
    supply_depot_data: dict[int, SupplyDepotData]
    marine_data: dict[int, MarineData]
    build_times: dict[UnitTypeId, int]
    expansion_locations: dict[Point2, bool]
    completed_bases = set()

    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.worker_data = {}
        self.townhall_data = {}
        self.gas_data = {}
        self.barracks_data = {}
        self.supply_depot_data = {}
        self.marine_data = {}
        self.expansion_locations = {el: el.distance_to(self.bot.start_location) < 15
            for el in self.bot.expansion_locations_list}
        self.worker_data = {worker.tag: WorkerData(WorkerRole.IDLE, worker.tag)
            for worker in self.bot.workers}
        self.townhall_data = {townhall.tag: TownhallData(townhall.position, townhall.tag)
            for townhall in self.bot.townhalls}
        self.gas_data = {geyser.tag: GasBuildingData(geyser.position, geyser.tag)
            for geyser in self.bot.gas_buildings}
        self.barracks_data = {barracks.tag: BarracksData(barracks.position, barracks.tag)
            for barracks in self.bot.structures.filter(lambda s: s.type_id == UnitTypeId.BARRACKS)}
        self.supply_depot_data = {depot.tag: SupplyDepotData(depot.position, depot.tag)
            for depot in self.bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT)}
        self.marine_data = {marine.tag: MarineData(marine.tag)
            for marine in self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE)}
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
            self.handle_worker_destroyed(tag)
        elif tag in self.townhall_data:
            self.handle_townhall_destroyed(tag)
        elif tag in self.gas_data:
            self.handle_gas_destroyed(tag)
        elif tag in self.barracks_data:
            self.handle_barracks_destroyed(tag)
        elif tag in self.supply_depot_data:
            self.handle_supply_depot_destroyed(tag)
        elif tag in self.marine_data:
            self.handle_marine_destroyed(tag)

    def handle_worker_destroyed(self, tag: int) -> None:
        # remove assigned worker from assigned structure
        if self.worker_data[tag].assigned_to_tag is not None:
            if self.worker_data[tag].role == WorkerRole.MINERALS:
                self.townhall_data[self.worker_data[tag].assigned_to_tag].current_harvesters -= 1
            elif self.worker_data[tag].role == WorkerRole.GAS:
                self.gas_data[self.worker_data[tag].assigned_to_tag].current_harvesters -= 1
        # if the worker was on the way to build a base, make the location available
        if self.bot.base_worker and self.bot.base_worker.tag == tag:
            self.expansion_locations[self.bot.new_base_location] = False
            self.bot.base_worker = None
            self.bot.new_base_location = None
        # if the worker was building a base, make the location available
        worker = self.bot._units_previous_map[tag]
        for order in worker.orders:
            if order.ability.id == AbilityId.TERRANBUILD_COMMANDCENTER:
                self.expansion_locations[order.target] = False
        self.worker_data.pop(tag)

    def handle_townhall_destroyed(self, tag: int) -> None:
        # remove all assigned workers from the townhall, make location available
        for worker in self.worker_data.values():
            if worker.assigned_to_tag == tag:
                self.bot.worker_manager.assign_worker(worker.tag, WorkerRole.IDLE, None)
        self.expansion_locations[self.townhall_data[tag].position] = False
        if tag in self.completed_bases:
            self.completed_bases.remove(tag)
        self.townhall_data.pop(tag)

    def handle_gas_destroyed(self, tag: int) -> None:
        # remove all assigned workers from the gas geyser, make location available
        for worker in self.worker_data.values():
            if worker.assigned_to_tag == tag:
                self.bot.worker_manager.assign_worker(worker.tag, WorkerRole.IDLE, None)
        # the tag of the closest townhall
        position = self.gas_data[tag].position
        closest_th_tag = min(self.townhall_data, key=lambda th: position.distance_to(self.townhall_data[th].position))
        # if the townhall is close enough, remove the base from the completed bases
        if position.distance_to(self.townhall_data[closest_th_tag].position) < 15:
            self.completed_bases.remove(closest_th_tag)
        self.gas_data.pop(tag)

    def handle_barracks_destroyed(self, tag: int) -> None:
        self.barracks_data.pop(tag)

    def handle_supply_depot_destroyed(self, tag: int) -> None:
        self.supply_depot_data.pop(tag)

    def handle_marine_destroyed(self, tag: int) -> None:
        self.marine_data.pop(tag)