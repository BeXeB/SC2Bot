import math
import threading

from sc2.data import Result
from sc2.position import Point2
from sc2_mcts import *

from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId
from sc2.ids.ability_id import AbilityId
from sc2.unit import Unit

from Actions.build_worker import WorkerBuilder
from state_translator import translate_state
from result_saver import save_result

from Actions.BuildBase import BaseBuilder
from Actions.VespeneExtractor import VespeneBuilder
from Actions.build_supply import SupplyBuilder

from worker_manager import WorkerManager, TownhallData, GasBuildingData, WorkerRole

STEPS_PER_SECOND = 22.4

class MyBot(BotAI):
    next_action: Action = Action.none
    # Completed_bases is used to keep track of processed vespene extractors
    def __init__(self,
                 mcts_seed: int = 0,
                 mcts_rollout_end_time: int = 300,
                 mcts_exploration: float = math.sqrt(2),
                 mcts_value_heuristics: ValueHeuristic = ValueHeuristic.UCT,
                 mcts_rollout_heuristics: RolloutHeuristic = RolloutHeuristic.weighted_choice) -> None:
        self.completed_bases = set()
        self.busy_workers: dict[int, float] = {}
        self.mcts = Mcts(State(), mcts_seed, mcts_rollout_end_time, mcts_exploration, mcts_value_heuristics, mcts_rollout_heuristics)
        self.mcts_settings = [
            mcts_seed,
            mcts_rollout_end_time,
            mcts_exploration,
            mcts_value_heuristics,
            mcts_rollout_heuristics,
        ]
        self.actions_taken: dict[int, Action] = {}

    async def on_start(self):
        # self.CC_BUILD_TIME_STEPS: int = self.game_data.units[UnitTypeId.COMMANDCENTER.value]._proto.build_time
        # self.CC_TRAVEL_TIME_STEPS: int = math.ceil(5 * STEPS_PER_SECOND)
        # self.REFINERY_BUILD_TIME_STEPS: int = self.game_data.units[UnitTypeId.REFINERY.value]._proto.build_time
        # self.REFINERY_TRAVEL_TIME_STEPS: int = math.ceil(1 * STEPS_PER_SECOND)
        # self.WORKER_BUILD_TIME_STEPS: int = self.game_data.units[UnitTypeId.SCV.value]._proto.build_time
        # self.SUPPLY_BUILD_TIME_STEPS: int = self.game_data.units[UnitTypeId.SUPPLYDEPOT.value]._proto.build_time
        # self.SUPPLY_TRAVEL_TIME_STEPS: int = math.ceil(2 * STEPS_PER_SECOND)
        self.CC_BUILD_TIME_SECONDS: int = math.ceil(self.game_data.units[UnitTypeId.COMMANDCENTER.value]._proto.build_time / STEPS_PER_SECOND)
        self.CC_TRAVEL_TIME_SECONDS: int = 5
        self.REFINERY_BUILD_TIME_SECONDS: int = math.ceil(self.game_data.units[UnitTypeId.REFINERY.value]._proto.build_time / STEPS_PER_SECOND)
        self.REFINERY_TRAVEL_TIME_SECONDS: int = 1
        self.WORKER_BUILD_TIME_SECONDS: int = math.ceil(self.game_data.units[UnitTypeId.SCV.value]._proto.build_time / STEPS_PER_SECOND)
        self.SUPPLY_BUILD_TIME_SECONDS: int = math.ceil(self.game_data.units[UnitTypeId.SUPPLYDEPOT.value]._proto.build_time / STEPS_PER_SECOND)
        self.SUPPLY_TRAVEL_TIME_SECONDS: int = 2
        self.base_builder = BaseBuilder(self)
        self.vespene_builder = VespeneBuilder(self)
        self.supply_builder = SupplyBuilder(self)
        self.worker_manager = WorkerManager(self)
        self.worker_builder = WorkerBuilder(self)
        self.mcts.start_search()

    async def on_step(self, iteration: int) -> None:
        if iteration == 0:
            for worker in self.workers:
                worker(AbilityId.STOP_STOP)
            for townhall in self.townhalls:
                townhall(AbilityId.RALLY_WORKERS, self.start_location)

        self.update_busy_workers()
        self.manage_workers()


        match self.next_action:
            case Action.build_base:
                if not self.can_afford(UnitTypeId.COMMANDCENTER):
                    return
                new_base_location = await self.base_builder.find_next_base_location()
                self.build_base(new_base_location)
                self.actions_taken.update({iteration: Action.build_base})
                self.set_next_action()
            case Action.build_vespene_collector:
                if not self.can_afford(UnitTypeId.REFINERY):
                    return
                th = self.townhalls.random
                # TODO if we are unable to place an extractor to this base try another one
                if th.tag not in self.completed_bases and th.is_ready:
                    await self.vespene_builder.build_vespene_extractor(self.townhalls.random.position)
                    # TODO only add to completed if all the vespene extractors are built
                    self.completed_bases.add(th.tag)
                self.actions_taken.update({iteration: Action.build_vespene_collector})
                self.set_next_action()
            case Action.build_worker:
                if not self.can_afford(UnitTypeId.SCV):
                    return
                # TODO if we are unable to build an scv, wait
                await self.worker_builder.build_worker()
                self.actions_taken.update({iteration: Action.build_worker})
                self.set_next_action()
            case Action.build_house:
                if not self.can_afford(UnitTypeId.SUPPLYDEPOT):
                    return
                await self.supply_builder.build_supply()
                self.actions_taken.update({iteration: Action.build_house})
                self.set_next_action()
            case Action.none:
                self.set_next_action(self.mcts.get_best_action())
                self.mcts.update_root_state(translate_state(self))
                print(self.next_action)


    def set_next_action(self, action: Action = Action.none):
        # with self.next_action_mutex:
        self.next_action = action

    async def on_building_construction_complete(self, unit: Unit) -> None:
        if unit.type_id == UnitTypeId.COMMANDCENTER:
            self.worker_manager.th_data.update({unit.tag: TownhallData()})
        if unit.type_id == UnitTypeId.REFINERY:
            self.worker_manager.gas_data.update({unit.tag: GasBuildingData()})
        if unit.type_id == UnitTypeId.SUPPLYDEPOT:
            unit(AbilityId.MORPH_SUPPLYDEPOT_LOWER)
        building_worker = self.workers.closest_to(unit)
        self.worker_manager.assign_worker(building_worker.tag, WorkerRole.IDLE, None)

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

    def build_base(self, position: Point2):
        if position:
            worker = self.worker_manager.select_worker(position, WorkerRole.BUILD)
            worker.build(UnitTypeId.COMMANDCENTER, position)
            self.busy_workers.update({worker.tag: self.CC_BUILD_TIME_SECONDS + self.CC_TRAVEL_TIME_SECONDS})

    async def on_end(self, game_result: Result):
        self.mcts.stop_search()
        end_state = translate_state(self)
        save_result(self, end_state, self.time)

class PeacefulBot(BotAI):
    async def on_step(self, iteration: int) -> None:
        if self.time > 1 * 60:
            await self.client.quit()