import math
import queue
from enum import Enum
from itertools import product
from typing import Tuple
from time import sleep

from Python.Actions.scoutmanager import ScoutManager
from sc2.data import Result
from sc2.position import Point3, Point2
from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId
from sc2.ids.ability_id import AbilityId
from sc2.unit import Unit

from Python.Modules.map_analyzer import MapAnalyzer
from sc2_mcts import *
from Python.Actions.build_barracks import BarracksBuilder
from Python.Actions.build_marine import MarineBuilder
from Python.Actions.BuildBase import BaseBuilder
from Python.Actions.VespeneExtractor import VespeneBuilder
from Python.Actions.build_supply import SupplyBuilder
from Python.Actions.build_worker import WorkerBuilder
from Python.Modules.state_translator import translate_state
from Python.Modules.result_saver import save_result
from Python.Modules.worker_manager import WorkerManager
from Python.Modules.army_manager import ArmyManager
from Python.Modules.information_manager import WorkerRole, TownhallData, GasBuildingData, InformationManager, \
    SupplyDepotData, BarracksData, STEPS_PER_SECOND, WorkerData, MarineData, PlacementType


class ActionSelection(Enum):
    BestAction = 0
    BestActionMin = 1
    MultiBestAction = 2
    MultiBestActionMin = 3

# TODO: Fix vespene extractor location finding, somehow we run out of locations with a lot of them being available
# TODO: Fix the worker selection, sometimes workers that are already on the way to build something are selected
# TODO: Better build supply
# TODO: Better build barracks
# TODO: Save replay option

class MyBot(BotAI):
    information_manager: InformationManager
    worker_manager: WorkerManager
    base_builder: BaseBuilder
    vespene_builder: VespeneBuilder
    supply_builder: SupplyBuilder
    worker_builder: WorkerBuilder
    barracks_builder: BarracksBuilder
    marine_builder: MarineBuilder
    army_manager: ArmyManager
    map_analyzer: MapAnalyzer
    scout_manager: ScoutManager
    new_base_location = None
    base_worker = None
    busy_workers: dict[int, float] = {}
    actions_taken: dict[int, Action] = {}

    # Completed_bases is used to keep track of processed vespene extractors
    def __init__(self,
                 mcts_seed: int = 0,
                 mcts_rollout_end_time: int = 300,
                 mcts_exploration: float = math.sqrt(2),
                 mcts_value_heuristics: ValueHeuristic = ValueHeuristic.UCT,
                 mcts_rollout_heuristics: RolloutHeuristic = RolloutHeuristic.weighted_choice,
                 time_limit: int = 600,
                 action_selection: ActionSelection = ActionSelection.BestAction,
                 future_action_queue_length: int = 1,
                 minimum_search_rollouts: int = 5000) -> None:
        self.mcts = Mcts(State(), mcts_seed, mcts_rollout_end_time, mcts_exploration, mcts_value_heuristics, mcts_rollout_heuristics)
        self.mcts_settings = [
            mcts_seed,
            mcts_rollout_end_time,
            mcts_exploration,
            mcts_value_heuristics,
            mcts_rollout_heuristics,
        ]
        self.time_limit = time_limit
        self.action_selection = action_selection
        self.fixed_search_rollouts = minimum_search_rollouts
        self.next_action: Action = Action.none
        self.future_action_queue: queue.Queue = queue.Queue(maxsize=future_action_queue_length)

    async def on_start(self):
        self.information_manager = InformationManager(self)
        self.worker_manager = WorkerManager(self)
        self.base_builder = BaseBuilder(self)
        self.vespene_builder = VespeneBuilder(self)
        self.supply_builder = SupplyBuilder(self)
        self.worker_builder = WorkerBuilder(self)
        self.barracks_builder = BarracksBuilder(self)
        self.marine_builder = MarineBuilder(self)
        self.army_manager = ArmyManager(self)
        self.map_analyzer = MapAnalyzer(self)
        self.scout_manager = ScoutManager(self)
        self.mcts.start_search()

    async def on_step(self, iteration: int) -> None:
        if iteration == 0:
            #await self.client.debug_show_map()
            self.map_analyzer.setup_grid()
            self.map_analyzer.print()
            for worker in self.workers:
                worker(AbilityId.STOP_STOP)
            for townhall in self.townhalls:
                townhall(AbilityId.RALLY_WORKERS, self.start_location)

        # await self.draw_debug()

        self.update_busy_workers()
        self.manage_workers()
        self.army_manager.manage_army()
        self.army_manager.check_base_radius()
        self.scout_manager.manage_scouts()

        match self.next_action:
            case Action.build_base:
                await self.build_base()
                self.actions_taken.update({iteration: Action.build_base})
            case Action.build_vespene_collector:
                await self.build_vespene_collector()
                self.actions_taken.update({iteration: Action.build_vespene_collector})
            case Action.build_worker:
                await self.build_worker()
                self.actions_taken.update({iteration: Action.build_worker})
            case Action.build_house:
                await self.build_house()
                self.actions_taken.update({iteration: Action.build_house})
            case Action.build_barracks:
                await self.build_barracks()
                self.actions_taken.update({iteration: Action.build_barracks})
            case Action.build_marine:
                await self.build_marine()
                self.actions_taken.update({iteration: Action.build_marine})
            case Action.none:
                match self.action_selection:
                    case ActionSelection.BestAction:
                        self.get_best_action()
                    case ActionSelection.BestActionMin:
                        self.get_best_action_min()
                    case ActionSelection.MultiBestAction:
                        self.get_multi_best_action()
                    case ActionSelection.MultiBestActionMin:
                        self.get_multi_best_action_min()

    async def draw_debug(self):
        def draw_box(loc: Point2, start_loc: Tuple[int, int], end_loc: Tuple[int, int], color:Tuple[int, int, int]):
            height = self.get_terrain_z_height(loc) + 0.1
            self.client.debug_box_out(Point3((start_loc[0], start_loc[1], height)), Point3((end_loc[0], end_loc[1], height)), color=color)

        grid = self.map_analyzer.grid
        for x, y in product(range(grid.width), range(grid.height)):
                if grid[(x, y)]:
                    self.client.debug_text_3d(f"{x},{y}", Point3((x, y+0.5, self.get_terrain_z_height(Point2((x,y)))+0.1)), (255, 255, 255))

        thlocs = self.information_manager.expansion_locations
        for thloc in thlocs:
            color = (255, 0, 0) if self.information_manager.expansion_locations[thloc] else (0, 255, 0)
            start_loc = (math.floor(thloc.x)-2, math.floor(thloc.y)-2)
            end_loc = (start_loc[0]+5, start_loc[1]+5)
            draw_box(thloc, start_loc, end_loc, color)

    async def build_barracks(self) -> None:
        if not self.can_afford(UnitTypeId.BARRACKS):
            return
        if not self.tech_requirement_progress(UnitTypeId.BARRACKS) >= 1:
            return
        await self.barracks_builder.build_barracks()
        self.set_next_action()

    async def build_marine(self) -> None:
        if not self.can_afford(UnitTypeId.MARINE):
            return
        if not self.structures.filter(lambda sr: sr.type_id == UnitTypeId.BARRACKS).ready.filter(lambda t: len(t.orders) == 0):
            return
        await self.marine_builder.build_marine()
        self.set_next_action()

    async def build_base(self) -> None:
        if not self.new_base_location:
            self.new_base_location = await self.base_builder.find_next_base_location()
            if not self.new_base_location:
                self.set_next_action()
                print("Unable to find free base location")
                return
            self.base_worker = self.worker_manager.select_worker(self.new_base_location, WorkerRole.BUILD)
            if not self.base_worker:
                self.set_next_action()
                return
            self.base_worker.move(self.new_base_location)
        if not self.can_afford(UnitTypeId.COMMANDCENTER):
            return
        self.base_worker.build(UnitTypeId.COMMANDCENTER, self.new_base_location, queue=True)
        self.information_manager.expansion_locations[self.new_base_location] = True
        self.busy_workers.update({self.base_worker.tag: self.information_manager.build_times[UnitTypeId.COMMANDCENTER]})
        self.new_base_location = None
        self.base_worker = None
        self.set_next_action()

    async def build_vespene_collector(self) -> None:
        if not self.can_afford(UnitTypeId.REFINERY):
            return
        available_ths = self.townhalls.filter(lambda t: t.tag not in self.information_manager.completed_bases)
        if not available_ths:
            self.set_next_action()
            print("Unable to find free vespene location")
            return
        townhall = available_ths.first
        if townhall.tag not in self.information_manager.completed_bases and townhall.is_ready:
            await self.vespene_builder.build_vespene_extractor(self.townhalls.random.position)
            if len(self.vespene_geyser.closer_than(10, townhall.position)) == len(
                    self.gas_buildings.closer_than(10, townhall.position)) + 1:
                self.information_manager.completed_bases.add(townhall.tag)
        self.set_next_action()

    async def build_worker(self) -> None:
        if not self.can_afford(UnitTypeId.SCV, check_supply_cost=False):
            return
        if not self.can_afford(UnitTypeId.SCV):
            self.set_next_action()
            return
        if not self.townhalls.ready.filter(lambda t: len(t.orders) == 0):
            return
        await self.worker_builder.build_worker()
        self.set_next_action()

    async def build_house(self) -> None:
        if not self.can_afford(UnitTypeId.SUPPLYDEPOT):
            return
        await self.supply_builder.build_supply()
        self.set_next_action()

    async def on_unit_destroyed(self, unit_tag: int) -> None:
        await self.information_manager.remove_unit_by_tag(unit_tag)

    async def on_building_construction_complete(self, unit: Unit) -> None:
        match unit.type_id:
            case UnitTypeId.COMMANDCENTER:
                self.information_manager.townhall_data.update({unit.tag: TownhallData(unit.position, unit.tag)})
            case UnitTypeId.REFINERY:
                self.information_manager.gas_data.update({unit.tag: GasBuildingData(unit.position, unit.tag)})
            case UnitTypeId.SUPPLYDEPOT:
                unit(AbilityId.MORPH_SUPPLYDEPOT_LOWER)
                self.information_manager.supply_depot_data.update({unit.tag: SupplyDepotData(unit.position, unit.tag)})
            case UnitTypeId.BARRACKS:
                self.information_manager.barracks_data.update({unit.tag: BarracksData(unit.position, unit.tag)})
        building_worker = self.workers.closest_to(unit)
        self.worker_manager.assign_worker(building_worker.tag, WorkerRole.IDLE, None)

    async def on_unit_created(self, unit: Unit):
        match unit.type_id:
            case UnitTypeId.SCV:
                unit(AbilityId.STOP_STOP)
                self.information_manager.worker_data.update({unit.tag: WorkerData(WorkerRole.IDLE, unit.tag)})
            case UnitTypeId.MARINE:
                self.information_manager.marine_data.update({unit.tag: MarineData(unit.tag)})

    async def on_end(self, game_result: Result):
        self.mcts.stop_search()
        end_state = translate_state(self)
        save_result(self, end_state, self.time)
        self.future_action_queue.queue.clear()

    def get_best_action(self) -> None:
        print(self.mcts.get_number_of_rollouts())
        action = self.mcts.get_best_action()
        self.set_next_action(action)
        state = translate_state(self)
        self.mcts.update_root_state(state)
        self.mcts.perform_action(action)

    def get_best_action_min(self) -> None:
        while self.mcts.get_number_of_rollouts() < self.fixed_search_rollouts:
            sleep(0.01)
        self.get_best_action()

    def get_multi_best_action(self) -> None:
        if not self.future_action_queue.empty():
            self.set_next_action(self.future_action_queue.get())
            return
        print(self.mcts.get_number_of_rollouts())
        action = self.mcts.get_best_action()
        self.mcts.perform_action(action)
        for i in range(self.future_action_queue.maxsize):
            a = self.mcts.get_best_action()
            self.mcts.perform_action(a)
            self.future_action_queue.put(a)
        state = translate_state(self)
        self.mcts.update_root_state(state)
        self.mcts.perform_action(action)
        for a in list(self.future_action_queue.queue):
            self.mcts.perform_action(a)
        self.set_next_action(action)

    def get_multi_best_action_min(self) -> None:
        if not self.future_action_queue.empty():
            self.set_next_action(self.future_action_queue.get())
            return
        while self.mcts.get_number_of_rollouts() < self.fixed_search_rollouts:
            sleep(0.01)
        self.get_multi_best_action()

    def set_next_action(self, action: Action = Action.none):
        self.next_action = action
        if action is not Action.none:
            print(action)

    # Update the busy workers for the state translator
    def update_busy_workers(self) -> None:
        workers_to_remove = []
        for worker in self.busy_workers:
            self.busy_workers[worker] -= 1 / (STEPS_PER_SECOND / 4)
            if self.busy_workers[worker] <= 0:
                workers_to_remove.append(worker)
        for worker in workers_to_remove:
            self.busy_workers.pop(worker)

    # Manage the workers, distribute them to the correct roles, and speed mine
    def manage_workers(self):
        self.worker_manager.distribute_workers()
        self.worker_manager.speed_mine()

class PeacefulBot(BotAI):
    async def on_step(self, iteration: int) -> None:
        # if iteration == 0:
        #     await self.client.debug_create_unit([[UnitTypeId.MARINE, 10, Point2((self.enemy_start_locations[0].x+10, self.enemy_start_locations[0].y+10)), 2]])
        try:
            if self.can_afford(UnitTypeId.SUPPLYDEPOT) and self.supply_left < 5:
                await self.build(UnitTypeId.SUPPLYDEPOT, near=self.townhalls.random)
            if self.can_afford(UnitTypeId.BARRACKS) and self.structures(UnitTypeId.BARRACKS).amount < 3:
                await self.build(UnitTypeId.BARRACKS, near=self.townhalls.random)
            if self.can_afford(UnitTypeId.MARINE):
                self.train(UnitTypeId.MARINE)
            await self.distribute_workers()
        except:
            return