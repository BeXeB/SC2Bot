import math
import queue
from enum import Enum
from typing import Optional

from sc2.data import Result
from sc2.position import Point2, Point3

from Actions.build_barracks import BarracksBuilder
from Actions.build_marine import MarineBuilder
from sc2_mcts import *

from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId
from sc2.ids.ability_id import AbilityId
from sc2.unit import Unit

from Actions.build_worker import WorkerBuilder
from Modules.state_translator import translate_state
from Modules.result_saver import save_result

from Actions.BuildBase import BaseBuilder
from Actions.VespeneExtractor import VespeneBuilder
from Actions.build_supply import SupplyBuilder

from Modules.worker_manager import WorkerManager
from Python.Modules.information_manager import WorkerRole, TownhallData, GasBuildingData, InformationManager

STEPS_PER_SECOND = 22.4

class ActionSelection(Enum):
    BestAction = 0
    BestActionFixed = 1
    MultiBestAction = 2
    MultiBestActionFixed = 3
    MultiBestActionMin = 4

# TODO: Handle losing bases and workers on their way to build
# TODO: Worker manager might not handle bases and workers dying
# TODO: Dont build bases at base locations with no minerals
# TODO: Refactor the worker manager
# TODO: Refactor StateTranslator
# TODO: Save replay option
# TODO: Better build supply

class MyBot(BotAI):
    information_manager: InformationManager
    worker_manager: WorkerManager
    base_builder: BaseBuilder
    vespene_builder: VespeneBuilder
    supply_builder: SupplyBuilder
    worker_builder: WorkerBuilder
    barracks_builder: BarracksBuilder
    marine_builder: MarineBuilder
    new_base_location = None
    base_worker = None
    completed_bases = set()
    busy_workers: dict[int, float] = {}
    el_list: dict[Point2, bool] = {}
    actions_taken: dict[int, Action] = {}
    build_times: dict[UnitTypeId, int] = {}

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
                 fixed_search_rollouts: int = 5000) -> None:
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
        self.fixed_search_rollouts = fixed_search_rollouts
        self.next_action: Action = Action.none
        self.future_action_queue: queue.Queue = queue.Queue(maxsize=future_action_queue_length)

    async def on_start(self):
        self.build_times = {
            UnitTypeId.COMMANDCENTER: math.ceil(self.game_data.units[UnitTypeId.COMMANDCENTER.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.REFINERY: math.ceil(self.game_data.units[UnitTypeId.REFINERY.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.SCV: math.ceil(self.game_data.units[UnitTypeId.SCV.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.SUPPLYDEPOT: math.ceil(self.game_data.units[UnitTypeId.SUPPLYDEPOT.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.BARRACKS: math.ceil(self.game_data.units[UnitTypeId.BARRACKS.value]._proto.build_time / STEPS_PER_SECOND),
            UnitTypeId.MARINE: math.ceil(self.game_data.units[UnitTypeId.MARINE.value]._proto.build_time / STEPS_PER_SECOND),
        }
        self.information_manager = InformationManager(self)
        self.worker_manager = WorkerManager(self)
        self.base_builder = BaseBuilder(self)
        self.vespene_builder = VespeneBuilder(self)
        self.supply_builder = SupplyBuilder(self)
        self.worker_builder = WorkerBuilder(self)
        self.barracks_builder = BarracksBuilder(self)
        self.marine_builder = MarineBuilder(self)
        for el in self.expansion_locations_list:
            self.el_list.update({el: False})
            if el.distance_to(self.start_location) < 15:
                self.el_list[el] = True
        if self.action_selection is ActionSelection.MultiBestActionFixed or self.action_selection is ActionSelection.BestActionFixed:
            self.mcts.start_search_rollout(self.fixed_search_rollouts)
        else:
            self.mcts.start_search()

    async def on_step(self, iteration: int) -> None:
        if iteration == 0:
            for worker in self.workers:
                worker(AbilityId.STOP_STOP)
            for townhall in self.townhalls:
                townhall(AbilityId.RALLY_WORKERS, self.start_location)

        await self.draw_debug()

        self.update_busy_workers()
        self.manage_workers()

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
                try:
                    match self.action_selection:
                        case ActionSelection.BestAction:
                            self.get_best_action()
                        case ActionSelection.BestActionFixed:
                            self.get_best_action_fixed()
                        case ActionSelection.MultiBestAction:
                            self.get_multi_best_action()
                        case ActionSelection.MultiBestActionFixed:
                            self.get_multi_best_action_fixed()
                        case ActionSelection.MultiBestActionMin:
                            self.get_multi_best_action_min()
                except:
                    return

    async def draw_debug(self):
        blocs = self.barracks_builder.build_locations
        for bloc in blocs:
            height = self.get_terrain_z_height(bloc) + 0.1
            self.client.debug_sphere_out(Point3((bloc.x, bloc.y, height)), 1.5, (255, 0, 0))

        slocs = self.supply_builder.possible_supply_positions
        for sloc in slocs:
            height = self.get_terrain_z_height(sloc) + 0.1
            self.client.debug_sphere_out(Point3((sloc.x, sloc.y, height)), 1, (0, 255, 0))

    async def build_barracks(self) -> None:
        if not self.can_afford(UnitTypeId.BARRACKS):
            return
        await self.barracks_builder.build_barracks()
        self.set_next_action()

    async def build_marine(self) -> None:
        if not self.can_afford(UnitTypeId.MARINE):
            return
        await self.marine_builder.build_marine()
        self.set_next_action()

    async def build_base(self) -> None:
        if not self.new_base_location:
            self.new_base_location = await self.base_builder.find_next_base_location()
            if not self.new_base_location:
                self.set_next_action()
                return
            self.base_worker = self.worker_manager.select_worker(self.new_base_location, WorkerRole.BUILD)
            self.base_worker.move(self.new_base_location)
        if not self.can_afford(UnitTypeId.COMMANDCENTER):
            return
        self.base_worker.build(UnitTypeId.COMMANDCENTER, self.new_base_location, queue=True)
        self.el_list[self.new_base_location] = True
        self.busy_workers.update({self.base_worker.tag: self.build_times[UnitTypeId.COMMANDCENTER]})
        self.new_base_location = None
        self.base_worker = None
        self.set_next_action()

    async def build_vespene_collector(self) -> None:
        if not self.can_afford(UnitTypeId.REFINERY):
            return
        available_ths = self.townhalls.filter(lambda t: t.tag not in self.completed_bases)
        if not available_ths:
            self.set_next_action()
            return
        th = available_ths.first
        if th.tag not in self.completed_bases and th.is_ready:
            await self.vespene_builder.build_vespene_extractor(self.townhalls.random.position)
            if len(self.vespene_geyser.closer_than(10, th.position)) == len(
                    self.gas_buildings.closer_than(10, th.position)) + 1:
                self.completed_bases.add(th.tag)
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
        # If it is a worker that had an order do something:
        # I might be able to get this data from worker manager

        def __is_near_to_expansion(t):
            return t.distance_to(el) < self.EXPANSION_GAP_THRESHOLD
        # Iterate through el and see if an occupied base is free
        for el in self.el_list:
            is_occupied = self.el_list[el]
            if is_occupied and not any(map(__is_near_to_expansion, self.townhalls)):
                self.el_list[el] = False

    async def on_building_construction_complete(self, unit: Unit) -> None:
        # TODO Make this a switch case
        if unit.type_id == UnitTypeId.COMMANDCENTER:
            self.worker_manager.th_data.update({unit.tag: TownhallData()})
        if unit.type_id == UnitTypeId.REFINERY:
            self.worker_manager.gas_data.update({unit.tag: GasBuildingData()})
        if unit.type_id == UnitTypeId.SUPPLYDEPOT:
            unit(AbilityId.MORPH_SUPPLYDEPOT_LOWER)
        building_worker = self.workers.closest_to(unit)
        self.worker_manager.assign_worker(building_worker.tag, WorkerRole.IDLE, None)

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
        state.perform_action(action)
        self.mcts.update_root_state(state)

    def get_best_action_fixed(self) -> None:
        if self.mcts.get_number_of_rollouts() < self.fixed_search_rollouts:
            return
        self.get_best_action()
        self.mcts.stop_search()
        self.mcts.start_search_rollout(self.fixed_search_rollouts)

    def get_multi_best_action(self) -> None:
        if not self.future_action_queue.empty():
            self.set_next_action(self.future_action_queue.get())
            return
        print(self.mcts.get_number_of_rollouts())
        action = self.mcts.get_best_action()
        self.mcts.perform_action(action)
        for i in range(self.future_action_queue.maxsize):
            a = self.mcts.get_best_action()
            self.future_action_queue.put(a)
        state = translate_state(self)
        state.perform_action(action)
        for a in list(self.future_action_queue.queue):
            state.perform_action(a)
        self.set_next_action(action)
        self.mcts.update_root_state(state)

    def get_multi_best_action_fixed(self) -> None:
        if not self.future_action_queue.empty():
            self.set_next_action(self.future_action_queue.get())
            return
        if self.mcts.get_number_of_rollouts() < self.fixed_search_rollouts:
            return
        self.get_multi_best_action()
        self.mcts.stop_search()
        self.mcts.start_search_rollout(self.fixed_search_rollouts)

    def get_multi_best_action_min(self) -> None:
        if not self.future_action_queue.empty():
            self.set_next_action(self.future_action_queue.get())
            return
        if self.mcts.get_number_of_rollouts() < self.fixed_search_rollouts:
            return
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
        if self.can_afford(UnitTypeId.SUPPLYDEPOT) and self.supply_left < 5:
            await self.build(UnitTypeId.SUPPLYDEPOT, near=self.townhalls.random)
        if self.can_afford(UnitTypeId.BARRACKS) and self.structures(UnitTypeId.BARRACKS).amount < 3:
            await self.build(UnitTypeId.BARRACKS, near=self.townhalls.random)
        if self.can_afford(UnitTypeId.MARINE):
            self.train(UnitTypeId.MARINE)
        await self.distribute_workers()