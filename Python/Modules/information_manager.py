import math
import typing
from enum import Enum
from typing import Optional, Dict, Set, List, Tuple

from sc2.unit import Unit

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

STEPS_PER_SECOND = 22.4

class WorkerRole(Enum):
    IDLE = 0
    MINERALS = 1
    GAS = 2
    BUILD = 3
    SCOUT = 4

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

class FactoryData:
    position: Point2
    tag: int

    def __init__(self, position: Point2, tag: int) -> None:
        self.position = position
        self.tag = tag

class StarPortData:
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

class PlacementType(Enum):
    PRODUCTION = 0
    SUPPLY = 1
    TECH = 2

class InformationManager:
    worker_data: Dict[int, WorkerData]
    townhall_data: Dict[int, TownhallData]
    gas_data: Dict[int, GasBuildingData]
    barracks_data: Dict[int, BarracksData]
    supply_depot_data: Dict[int, SupplyDepotData]
    marine_data: Dict[int, MarineData]
    factory_data = Dict[int, FactoryData]
    starport_data = Dict[int, StarPortData]
    build_times: Dict[UnitTypeId, int]
    expansion_locations: Dict[Point2, bool]
    completed_bases: Set[int] = set()
    building_type_to_placement_type: Dict[UnitTypeId, PlacementType]
    placement_type_to_size: Dict[PlacementType, Tuple[int, int]]
    terranbuild_mapping: Dict[AbilityId, UnitTypeId]

    def __init__(self, bot: 'MyBot'):
        self.bot = bot
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
        self.factory_data = {fac.tag: FactoryData(fac.position, fac.tag)
            for fac in self.bot.structures.filter(lambda f: f.type_id == UnitTypeId.FACTORY)}
        self.starport_data = {starport.tag: StarPortData(starport.position, starport.tag)
            for starport in self.bot.structures.filter(lambda sp: sp.type_id == UnitTypeId.STARPORT)}
        self.supply_depot_data = {depot.tag: SupplyDepotData(depot.position, depot.tag)
            for depot in self.bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT)}
        self.marine_data = {marine.tag: MarineData(marine.tag)
            for marine in self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE)}
        self.build_times = {
            UnitTypeId.COMMANDCENTER: math.ceil(self.bot.game_data.units[UnitTypeId.COMMANDCENTER.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.REFINERY: math.ceil(self.bot.game_data.units[UnitTypeId.REFINERY.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.SCV: math.ceil(self.bot.game_data.units[UnitTypeId.SCV.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.SUPPLYDEPOT: math.ceil(self.bot.game_data.units[UnitTypeId.SUPPLYDEPOT.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.BARRACKS: math.ceil(self.bot.game_data.units[UnitTypeId.BARRACKS.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.MARINE: math.ceil(self.bot.game_data.units[UnitTypeId.MARINE.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.FACTORY: math.ceil(self.bot.game_data.units[UnitTypeId.FACTORY.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.STARPORT: math.ceil(self.bot.game_data.units[UnitTypeId.STARPORT.value].cost.time / STEPS_PER_SECOND)
        }
        self.units_to_ignore_for_army = {
            # Terran
            UnitTypeId.SCV,
            UnitTypeId.MULE,
            # Zerg
            UnitTypeId.DRONE,
            UnitTypeId.OVERLORD,
            UnitTypeId.LARVA,
            # Protoss
            UnitTypeId.PROBE
        }
        self.building_type_to_placement_type = {
            UnitTypeId.SUPPLYDEPOT: PlacementType.SUPPLY,
            UnitTypeId.BARRACKS: PlacementType.PRODUCTION,
            UnitTypeId.FACTORY: PlacementType.PRODUCTION,
            UnitTypeId.STARPORT: PlacementType.PRODUCTION,
            UnitTypeId.ARMORY: PlacementType.TECH,
            UnitTypeId.ENGINEERINGBAY: PlacementType.TECH,
            UnitTypeId.FUSIONCORE: PlacementType.TECH,
            UnitTypeId.GHOSTACADEMY: PlacementType.TECH,
        }
        self.placement_type_to_size = {
            PlacementType.SUPPLY: (2,2),
            PlacementType.PRODUCTION: (7,5),
            PlacementType.TECH: (3,3)
        }
        self.terranbuild_mapping = {
            AbilityId.TERRANBUILD_SUPPLYDEPOT: UnitTypeId.SUPPLYDEPOT,
            AbilityId.TERRANBUILD_BARRACKS: UnitTypeId.BARRACKS,
            AbilityId.TERRANBUILD_FACTORY: UnitTypeId.FACTORY,
            AbilityId.TERRANBUILD_STARPORT: UnitTypeId.STARPORT,
            AbilityId.TERRANBUILD_ARMORY: UnitTypeId.ARMORY,
            AbilityId.TERRANBUILD_GHOSTACADEMY: UnitTypeId.GHOSTACADEMY,
            AbilityId.TERRANBUILD_ENGINEERINGBAY: UnitTypeId.ENGINEERINGBAY,
            AbilityId.TERRANBUILD_FUSIONCORE: UnitTypeId.FUSIONCORE
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
        self.remove_worker_from_assigned_structure(tag)
        # if the worker was on the way to build a base, make the location available
        if self.bot.base_worker and self.bot.base_worker.tag == tag:
            self.expansion_locations[self.bot.new_base_location] = False
            self.bot.base_worker = None
            self.bot.new_base_location = None
        # if the worker was building a base, make the location available
        worker: Unit = self.bot._units_previous_map[tag]
        for order in worker.orders:
            p: Point2 = Point2.from_proto(order.target)
            ability: AbilityId = order.ability.id
            if ability == AbilityId.TERRANBUILD_COMMANDCENTER:
                self.expansion_locations[p] = False
            elif ability in self.terranbuild_mapping:
                placement_type = self.building_type_to_placement_type[self.terranbuild_mapping[ability]]
                self.bot.map_analyzer.make_location_buildable(p, placement_type)

        self.worker_data.pop(tag)

    def remove_worker_from_assigned_structure(self, tag: int) -> None:
        assigned_structure_tag: Optional[int] = self.worker_data[tag].assigned_to_tag
        if not assigned_structure_tag:
            return
        if self.worker_data[tag].role == WorkerRole.MINERALS and assigned_structure_tag in self.townhall_data:
            self.townhall_data[assigned_structure_tag].current_harvesters -= 1
        elif self.worker_data[tag].role == WorkerRole.GAS and assigned_structure_tag in self.gas_data:
            self.gas_data[assigned_structure_tag].current_harvesters -= 1

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
        if self.townhall_data:
            closest_th_tag = min(self.townhall_data, key=lambda th: position.distance_to(self.townhall_data[th].position))
            # if the townhall is close enough, remove the base from the completed bases
            if position.distance_to(self.townhall_data[closest_th_tag].position) < 15:
                if closest_th_tag in self.completed_bases:
                    self.completed_bases.remove(closest_th_tag)
        self.gas_data.pop(tag)

    def handle_barracks_destroyed(self, tag: int) -> None:
        if tag not in self.barracks_data:
            return
        position = self.barracks_data[tag].position
        self.bot.map_analyzer.make_location_buildable(position, self.building_type_to_placement_type[UnitTypeId.BARRACKS])
        self.barracks_data.pop(tag)

    def handle_supply_depot_destroyed(self, tag: int) -> None:
        if tag not in self.supply_depot_data:
            return
        position = self.supply_depot_data[tag].position
        self.bot.map_analyzer.make_location_buildable(position, self.building_type_to_placement_type[UnitTypeId.SUPPLYDEPOT])
        self.supply_depot_data.pop(tag)

    def handle_marine_destroyed(self, tag: int) -> None:
        self.marine_data.pop(tag)

    def get_workers(self, worker_role: Optional[WorkerRole]) -> dict[int, WorkerData]:
        if worker_role is None:
            return self.worker_data
        return {key: value for key, value in self.worker_data.items() if value.role == worker_role}