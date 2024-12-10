import math
from sc2_mcts import *

from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId
from sc2.ids.ability_id import AbilityId
from sc2.unit import Unit

from Actions.build_worker import WorkerBuilder
from state_translator import translate_state

from Actions.BuildBase import BaseBuilder
from Actions.VespeneExtractor import VespeneBuilder
from Actions.build_supply import SupplyBuilder

from worker_manager import WorkerManager, TownhallData, GasBuildingData, WorkerRole

STEPS_PER_SECOND = 22.4

class MyBot(BotAI):
    # Initializing our bot with the BaseBuilder, such that we don't initialize a new BaseBuilder instance on every step.
    # Furthermore, we initialize the vespene_builder
    # Completed_bases is used to keep track of processed vespene extractors
    def __init__(self) -> None:
        self.completed_bases = set()
        self.busy_workers: dict[int, float] = {}

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

    async def on_step(self, iteration: int) -> None:
        if iteration == 0:
            for worker in self.workers:
                worker(AbilityId.STOP_STOP)
            for townhall in self.townhalls:
                townhall(AbilityId.RALLY_WORKERS, self.start_location)

        translated_state = translate_state(self)
        print(translated_state)

        self.update_busy_workers()
        self.manage_workers()

        bases = []
        for base in translated_state.bases:
            Base(
                id=1,
                mineral_fields=base.number_of_mineral_fields,
                vespene_geysers=base.vespene_geysers,
                vespene_collectors=base.vespene_collectors,
            )
            bases.append(base)
        constructions = []
        for construction in translated_state.constructions:
            match construction.unit_type:
                case UnitTypeId.SUPPLYDEPOT:
                    action = Action.build_house
                case UnitTypeId.COMMANDCENTER:
                    action = Action.build_base
                case UnitTypeId.REFINERY:
                    action = Action.build_vespene_collector
                case UnitTypeId.SCV:
                    action = Action.build_worker
                case _:
                    action = Action.none
            Construction(
                time_left=construction.time_left,
                action=action
            )
            constructions.append(construction)
        state = state_builder(
            minerals=translated_state.minerals,
            vespene=translated_state.vespene,
            population=translated_state.population,
            incoming_population=translated_state.incoming_population,
            population_limit=translated_state.population_limit,
            occupied_worker_timers=translated_state.busy_workers,
            bases=bases,
            constructions=constructions
        )
        mcts = Mcts(state, 0, 100, math.sqrt(2), ValueHeuristic.UCT, RolloutHeuristic.weighted_choice)
        mcts.search(1000)
        best_action = mcts.get_best_action()

        # Bot code to automatically find next base location, and build a command center, if possible/affordable.
        # We'll have to update this later for the monte carlo tree search, but it will not be difficult whatsoever
        if self.can_afford(UnitTypeId.COMMANDCENTER) and not self.already_pending(UnitTypeId.COMMANDCENTER):
            new_base_location = await self.base_builder.find_next_base_location()
            if new_base_location:
                worker = self.worker_manager.select_worker(new_base_location, WorkerRole.BUILD)
                worker.build(UnitTypeId.COMMANDCENTER, new_base_location)
                # self.busy_workers.update({worker.tag: self.CC_BUILD_TIME_STEPS + self.CC_TRAVEL_TIME_STEPS})
                self.busy_workers.update({worker.tag: self.CC_BUILD_TIME_SECONDS + self.CC_TRAVEL_TIME_SECONDS})

        # Handle vespene extractor building for each completed base
        for townhall in self.townhalls:
            if townhall.tag not in self.completed_bases and townhall.is_ready:
                await self.vespene_builder.build_vespene_extractor(townhall.position)
                self.completed_bases.add(townhall.tag)

        if self.can_afford(UnitTypeId.SCV) and self.supply_left > 0:
            await self.worker_builder.build_worker()

        if self.can_afford(UnitTypeId.SUPPLYDEPOT) and self.supply_cap < 200:
            await self.supply_builder.build_supply()

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


class PeacefulBot(BotAI):
    async def on_step(self, iteration: int) -> None:
        # Gather minerals
        for worker in self.workers.idle:
            closest_minerals = self.mineral_field.closest_to(worker)
            worker.gather(closest_minerals)

        # Build a few units for defense if needed, but don't attack
        if self.can_afford(UnitTypeId.SCV) and not self.already_pending(UnitTypeId.SCV):
            self.train(UnitTypeId.SCV)