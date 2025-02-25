import typing
from enum import Enum
from typing import Optional

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class WorkerRole(Enum):
    IDLE = 0
    MINERALS = 1
    GAS = 2
    BUILD = 3

class TownhallData:
    current_harvesters: int
    position: Point2
    tag: int

    def __init__(self) -> None:
        self.current_harvesters = 0

class GasBuildingData:
    current_harvesters: int
    position: Point2
    tag: int

    def __init__(self) -> None:
        self.current_harvesters = 0

class BarracksData:
    position: Point2
    tag: int

    def __init__(self) -> None:
        pass

class SupplyDepotData:
    position: Point2
    tag: int

    def __init__(self) -> None:
        pass

class MarineData:
    position: Point2
    tag: int

    def __init__(self) -> None:
        pass

class WorkerData:
    assigned_to_tag: Optional[int]
    role: WorkerRole
    order_target: Optional[Point2]
    tag: int

    def __init__(self, role: WorkerRole) -> None:
        self.role = role
        self.assigned_to_tag = None

    def assign_to(self, tag: Optional[int], role: WorkerRole) -> None:
        self.assigned_to_tag = tag
        self.role = role

class InformationManager():
    worker_data: dict[int, WorkerData]
    th_data: dict[int, TownhallData]
    gas_data: dict[int, GasBuildingData]
    barracks_data: dict[int, BarracksData]
    supply_depot_data: dict[int, SupplyDepotData]
    marine_data: dict[int, MarineData]

    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.worker_data = {}
        self.th_data = {}
        self.gas_data = {}
        self.barracks_data = {}
        self.supply_depot_data = {}
        self.marine_data = {}
        for worker in self.bot.workers:
            self.worker_data.update({worker.tag: WorkerData(WorkerRole.IDLE)})
        for th in self.bot.townhalls:
            self.th_data.update({th.tag: TownhallData()})
        for geyser in self.bot.gas_buildings:
            self.gas_data.update({geyser.tag: GasBuildingData()})
        for barracks in self.bot.structures.filter(lambda s: s.type_id == UnitTypeId.BARRACKS):
            self.barracks_data.update({barracks.tag: BarracksData()})
        for depot in self.bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT):
            self.supply_depot_data.update({depot.tag: SupplyDepotData()})
        for marine in self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE):
            self.marine_data.update({marine.tag: MarineData()})


    async def remove_unit_by_tag(self, tag: int) -> None:
        if tag in self.worker_data:
            # remove assigned worker from assigned structure
            self.worker_data.pop(tag)
        elif tag in self.th_data:
            # remove all assigned workers from the townhall
            self.th_data.pop(tag)
        elif tag in self.gas_data:
            # remove all assigned workers from the gas geyser
            self.gas_data.pop(tag)
        elif tag in self.barracks_data:
            self.barracks_data.pop(tag)
        elif tag in self.supply_depot_data:
            self.supply_depot_data.pop(tag)
        elif tag in self.marine_data:
            self.marine_data.pop(tag)