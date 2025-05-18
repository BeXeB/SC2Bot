import math
import os
import typing
from collections import namedtuple
from enum import Enum
from typing import Optional, Dict, Set, List, Tuple
import pandas as pd
import torch

from sc2.unit import Unit

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2
from data.data_processing import ArenaNetwork
from sc2.units import Units

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

STEPS_PER_SECOND = 22.4

CombatPower = namedtuple('CombatPower', ['ground_power', 'air_power'])
ProductionPower = namedtuple('ProductionPower', ['ground_production', 'air_production'])
EnemyEntity = namedtuple('EnemyEntity', ['entity', 'last_seen'])

class WorkerRole(Enum):
    IDLE = 0
    MINERALS = 1
    GAS = 2
    BUILD = 3
    SCOUT = 4

class StructureData:
    position: Point2
    tag: int
    structure_type: UnitTypeId

    def __init__(self, position: Point2, tag: int, structure_type: UnitTypeId):
        self.position = position
        self.tag = tag
        self.structure_type = structure_type

# Townhall, GasBuilding
class TownhallData(StructureData):
    current_harvesters: int

    def __init__(self, position: Point2, tag: int) -> None:
        super().__init__(position, tag, UnitTypeId.COMMANDCENTER)
        self.current_harvesters = 0

class GasBuildingData(StructureData):
    current_harvesters: int

    def __init__(self, position: Point2, tag: int) -> None:
        super().__init__(position, tag, UnitTypeId.REFINERY)
        self.current_harvesters = 0

# Marine, SiegeTank, VikingFighter
class UnitData:
    tag: int
    unit_type: UnitTypeId

    def __init__(self, tag: int, unit_type: UnitTypeId) -> None:
        self.tag = tag
        self.unit_type = unit_type

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
    combat_model: ArenaNetwork
    worker_data: Dict[int, WorkerData]
    structures_data: Dict[int, StructureData]
    townhall_data: Dict[int, TownhallData]
    gas_data: Dict[int, GasBuildingData]
    unit_data: Dict[int, UnitData]
    build_times: Dict[UnitTypeId, int]
    expansion_locations: Dict[Point2, bool]
    completed_bases: Set[int] = set()
    building_type_to_placement_type: Dict[UnitTypeId, PlacementType]
    placement_type_to_size: Dict[PlacementType, Tuple[int, int]]
    terranbuild_mapping: Dict[AbilityId, UnitTypeId]
    enemy_units: Dict[int, EnemyEntity]
    enemy_structures: Dict[int, EnemyEntity]
    units_to_ignore_for_army: Set[UnitTypeId]
    combat_powers: Dict[UnitTypeId, CombatPower]
    production_powers: Dict[UnitTypeId, ProductionPower]

    def __init__(self, bot: 'MyBot'):
        self.bot = bot

        filepath = 'data/micro_arena.csv'
        if os.path.exists(filepath) and bot.game_mode is not bot.GameMode.micro_arena:
            # Columns names of the feature vector
            self.column_names = pd.read_csv(filepath).drop('result', axis=1).columns

            self.combat_model = ArenaNetwork(input_size=len(self.column_names))
            self.combat_model.load_state_dict(torch.load('data/arena_model.pth'))
            self.combat_model.eval()
        else:
            print('combat data not found.')

        if bot.game_mode != bot.GameMode.micro_arena:
            self.expansion_locations = {el: el.distance_to(self.bot.start_location) < 15
                for el in self.bot.expansion_locations_list}
        else:
            self.expansion_locations = {}
        self.worker_data = {worker.tag: WorkerData(WorkerRole.IDLE, worker.tag)
            for worker in self.bot.workers}

        self.structures_to_init = {UnitTypeId.BARRACKS, UnitTypeId.FACTORY, UnitTypeId.STARPORT, UnitTypeId.SUPPLYDEPOT}
        self.structure_list = self.bot.structures.filter(lambda s: s.type_id in self.structures_to_init)
        self.structures_data = {structure.tag: StructureData(structure.position, structure.tag, structure.type_id)
                    for structure in self.structure_list}
        self.unit_data = {}

        self.townhall_data = {townhall.tag: TownhallData(townhall.position, townhall.tag)
            for townhall in self.bot.townhalls}
        self.gas_data = {geyser.tag: GasBuildingData(geyser.position, geyser.tag)
            for geyser in self.bot.gas_buildings}

        self.build_times = {
            UnitTypeId.COMMANDCENTER: math.ceil(self.bot.game_data.units[UnitTypeId.COMMANDCENTER.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.REFINERY: math.ceil(self.bot.game_data.units[UnitTypeId.REFINERY.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.SCV: math.ceil(self.bot.game_data.units[UnitTypeId.SCV.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.SUPPLYDEPOT: math.ceil(self.bot.game_data.units[UnitTypeId.SUPPLYDEPOT.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.BARRACKS: math.ceil(self.bot.game_data.units[UnitTypeId.BARRACKS.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.MARINE: math.ceil(self.bot.game_data.units[UnitTypeId.MARINE.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.FACTORY: math.ceil(self.bot.game_data.units[UnitTypeId.FACTORY.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.STARPORT: math.ceil(self.bot.game_data.units[UnitTypeId.STARPORT.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.SIEGETANK: math.ceil(self.bot.game_data.units[UnitTypeId.SIEGETANK.value].cost.time / STEPS_PER_SECOND),
            UnitTypeId.VIKING: math.ceil(self.bot.game_data.units[UnitTypeId.VIKING.value].cost.time / STEPS_PER_SECOND),
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
        self.enemy_units = {}
        self.enemy_structures = {}
        self.combat_powers = {
            # Terran
            UnitTypeId.SCV: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.MARINE: CombatPower(ground_power=1, air_power=1),
            UnitTypeId.MARAUDER: CombatPower(ground_power=1.5, air_power=0),
            UnitTypeId.REAPER: CombatPower(ground_power=1, air_power=0),
            UnitTypeId.GHOST: CombatPower(ground_power=2, air_power=2),
            UnitTypeId.HELLION: CombatPower(ground_power=2, air_power=0),
            UnitTypeId.HELLIONTANK: CombatPower(ground_power=2, air_power=0),
            UnitTypeId.CYCLONE: CombatPower(ground_power=3, air_power=3),
            UnitTypeId.WIDOWMINE: CombatPower(ground_power=3, air_power=3),
            UnitTypeId.SIEGETANK: CombatPower(ground_power=5, air_power=0),
            UnitTypeId.SIEGETANKSIEGED: CombatPower(ground_power=10, air_power=0),
            UnitTypeId.VIKINGFIGHTER: CombatPower(ground_power=0, air_power=10),
            UnitTypeId.VIKINGASSAULT: CombatPower(ground_power=2, air_power=0),
            UnitTypeId.BANSHEE: CombatPower(ground_power=5, air_power=0),
            UnitTypeId.MEDIVAC: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.RAVEN: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.BATTLECRUISER: CombatPower(ground_power=10, air_power=10),
            UnitTypeId.LIBERATOR: CombatPower(ground_power=10, air_power=4),
            # Protoss
            UnitTypeId.PROBE: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.ZEALOT: CombatPower(ground_power=1.5, air_power=0),
            UnitTypeId.STALKER: CombatPower(ground_power=1.5, air_power=2),
            UnitTypeId.SENTRY: CombatPower(ground_power=0.5, air_power=0.5),
            UnitTypeId.ADEPT: CombatPower(ground_power=1.5, air_power=0),
            UnitTypeId.HIGHTEMPLAR: CombatPower(ground_power=4, air_power=4),
            UnitTypeId.DARKTEMPLAR: CombatPower(ground_power=5, air_power=0),
            UnitTypeId.ARCHON: CombatPower(ground_power=5, air_power=5),
            UnitTypeId.IMMORTAL: CombatPower(ground_power=8, air_power=0),
            UnitTypeId.COLOSSUS: CombatPower(ground_power=10, air_power=0),
            UnitTypeId.DISRUPTOR: CombatPower(ground_power=5, air_power=0),
            UnitTypeId.PHOENIX: CombatPower(ground_power=0, air_power=8),
            UnitTypeId.VOIDRAY: CombatPower(ground_power=6, air_power=6),
            UnitTypeId.CARRIER: CombatPower(ground_power=10, air_power=10),
            UnitTypeId.ORACLE: CombatPower(ground_power=4, air_power=0),
            UnitTypeId.WARPPRISM: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.MOTHERSHIP: CombatPower(ground_power=10, air_power=10),
            # Zerg
            UnitTypeId.DRONE: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.OVERLORD: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.OVERSEER: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.LARVA: CombatPower(ground_power=0, air_power=0),
            UnitTypeId.QUEEN: CombatPower(ground_power=2, air_power=2),
            UnitTypeId.ZERGLING: CombatPower(ground_power=0.5, air_power=0),
            UnitTypeId.BANELING: CombatPower(ground_power=3, air_power=0),
            UnitTypeId.ROACH: CombatPower(ground_power=3, air_power=0),
            UnitTypeId.RAVAGER: CombatPower(ground_power=4, air_power=0),
            UnitTypeId.HYDRALISK: CombatPower(ground_power=3, air_power=7),
            UnitTypeId.LURKERMP: CombatPower(ground_power=8, air_power=0),
            UnitTypeId.INFESTOR: CombatPower(ground_power=5, air_power=5),
            UnitTypeId.ULTRALISK: CombatPower(ground_power=10, air_power=0),
            UnitTypeId.SWARMHOSTMP: CombatPower(ground_power=3, air_power=0),
            UnitTypeId.LOCUSTMP: CombatPower(ground_power=3, air_power=0),
            UnitTypeId.BROODLORD: CombatPower(ground_power=10, air_power=0),
            UnitTypeId.BROODLING: CombatPower(ground_power=4, air_power=0),
            UnitTypeId.VIPER: CombatPower(ground_power=5, air_power=5),
            UnitTypeId.MUTALISK: CombatPower(ground_power=4, air_power=4),
            UnitTypeId.CORRUPTOR: CombatPower(ground_power=0, air_power=8),
        }
        self.production_powers = {
            # Terran
            UnitTypeId.BARRACKS: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.BARRACKSTECHLAB: ProductionPower(ground_production=1.5, air_production=0),
            UnitTypeId.BARRACKSREACTOR: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.FACTORY: ProductionPower(ground_production=1.5, air_production=0),
            UnitTypeId.FACTORYTECHLAB: ProductionPower(ground_production=3, air_production=1),
            UnitTypeId.FACTORYREACTOR: ProductionPower(ground_production=1.5, air_production=0),
            UnitTypeId.STARPORT: ProductionPower(ground_production=0.5, air_production=2),
            UnitTypeId.STARPORTTECHLAB: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.STARPORTREACTOR: ProductionPower(ground_production=0.5, air_production=1),
            UnitTypeId.GHOSTACADEMY: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.FUSIONCORE: ProductionPower(ground_production=2, air_production=2),
            # Protoss
            UnitTypeId.GATEWAY: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.WARPGATE: ProductionPower(ground_production=1.2, air_production=1.2),
            UnitTypeId.CYBERNETICSCORE: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.TWILIGHTCOUNCIL: ProductionPower(ground_production=1, air_production=1.2),
            UnitTypeId.TEMPLARARCHIVE: ProductionPower(ground_production=1.4, air_production=1),
            UnitTypeId.DARKSHRINE: ProductionPower(ground_production=1, air_production=0),
            UnitTypeId.ROBOTICSFACILITY: ProductionPower(ground_production=2, air_production=0),
            UnitTypeId.ROBOTICSBAY: ProductionPower(ground_production=1, air_production=0),
            UnitTypeId.STARGATE: ProductionPower(ground_production=1, air_production=2),
            UnitTypeId.FLEETBEACON: ProductionPower(ground_production=1, air_production=1.4),
            # Zerg
            UnitTypeId.HATCHERY: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.SPAWNINGPOOL: ProductionPower(ground_production=1, air_production=0),
            UnitTypeId.ROACHWARREN: ProductionPower(ground_production=1, air_production=0),
            UnitTypeId.BANELINGNEST: ProductionPower(ground_production=1, air_production=0),
            UnitTypeId.LAIR: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.HYDRALISKDEN: ProductionPower(ground_production=1, air_production=2),
            UnitTypeId.LURKERDEN: ProductionPower(ground_production=2, air_production=0),
            UnitTypeId.SPIRE: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.INFESTATIONPIT: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.HIVE: ProductionPower(ground_production=1, air_production=1),
            UnitTypeId.ULTRALISKCAVERN: ProductionPower(ground_production=2, air_production=0),
            UnitTypeId.GREATERSPIRE: ProductionPower(ground_production=1.5, air_production=1.5),
        }


    async def remove_unit_by_tag(self, tag: int) -> None:
        if tag in self.worker_data:
            self.handle_worker_destroyed(tag)
        elif tag in self.townhall_data:
            self.handle_townhall_destroyed(tag)
        elif tag in self.gas_data:
            self.handle_gas_destroyed(tag)
        elif tag in self.unit_data:
            self.handle_unit_destroyed(tag)
        elif tag in self.structures_data:
            self.handle_structure_destroyed(tag)
        elif tag in self.enemy_units:
            self.enemy_units.pop(tag)
        elif tag in self.enemy_structures:
            self.enemy_structures.pop(tag)

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
            if isinstance(order.target, int):
                continue
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

    def handle_unit_destroyed(self, tag: int) -> None:
        self.unit_data.pop(tag)

    def handle_structure_destroyed(self, tag: int) -> None:
        if tag not in self.structures_data:
            return
        if self.structures_data[tag].structure_type not in self.building_type_to_placement_type:
            return
        position = self.structures_data[tag].position
        self.bot.map_analyzer.make_location_buildable(position, self.building_type_to_placement_type[self.structures_data[tag].structure_type])

    def get_workers(self, worker_role: Optional[WorkerRole]) -> dict[int, WorkerData]:
        if worker_role is None:
            return self.worker_data
        return {key: value for key, value in self.worker_data.items() if value.role == worker_role}

    def get_combat_win_probability(self, player_units: Units, enemy_units: Units, on_creep: bool = False) -> float:
        features = self.__get_features(player_units, enemy_units, on_creep)
        with torch.no_grad():
            outputs = self.combat_model(features)
            probs = torch.exp(outputs) # Outputs is log probabilities, this converts them to regular probabilities
            win_prob = probs[0][2].item()

        return win_prob

    def __get_features(self, player_units: Units, enemy_units: Units, on_creep:bool) -> torch.Tensor:
        player_unit_dict = self.__get_units_for("player", player_units)
        enemy_unit_dict = self.__get_units_for("enemy", enemy_units)
        unit_dict = player_unit_dict | enemy_unit_dict | {"on_creep": 1 if on_creep else 0}

        df = pd.DataFrame([unit_dict])
        df = df[self.column_names] # only keep the columns which exist in the dataset
        row = df.iloc[0]

        tensor = torch.tensor(row.to_numpy(), dtype=torch.float32).unsqueeze(0)
        return tensor

    def __get_units_for(self, player: str,units: Units) -> dict:
        res: dict ={}

        for name in UnitTypeId._member_names_:
            res[player + ":" + name] = 0

        for unit in units:
            res[player + ":" + unit.type_id.name] += 1
        return res