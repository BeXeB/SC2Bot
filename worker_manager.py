from typing import Optional, Union

from sc2.bot_ai import BotAI
from enum import Enum

from sc2.ids.ability_id import AbilityId
from sc2.position import Point2
from sc2.unit import Unit
from sc2.units import Units

MINING_RADIUS = 1.325

class WorkerRole(Enum):
    IDLE = 0
    MINERALS = 1
    GAS = 2
    BUILD = 3

# TODO: move these to a centralized knowledge class
class TownhallData:
    current_harvesters: int

    def __init__(self) -> None:
        self.current_harvesters = 0

class GasBuildingData:
    current_harvesters: int

    def __init__(self) -> None:
        self.current_harvesters = 0

class WorkerData:
    assigned_to_tag: Optional[int]
    role: WorkerRole

    def __init__(self, role: WorkerRole) -> None:
        self.role = role
        self.assigned_to_tag = None

    def assign_to(self, tag: Optional[int], role: WorkerRole) -> None:
        self.assigned_to_tag = tag
        self.role = role

class WorkerManager:
    bot: BotAI
    mineral_targets: dict[int, Point2]
    worker_data: dict[int, WorkerData]
    th_data: dict[int, TownhallData]
    gas_data: dict[int, GasBuildingData]

    def __init__(self, bot: BotAI):
        self.bot = bot
        self.mineral_targets = {}
        self.worker_data = {}
        self.th_data = {}
        self.gas_data = {}
        self.calculate_mineral_targets()
        for worker in self.bot.workers:
            self.worker_data.update({worker.tag: WorkerData(WorkerRole.IDLE)})
        for th in self.bot.townhalls:
            self.th_data.update({th.tag: TownhallData()})
        for geyser in self.bot.gas_buildings:
            self.gas_data.update({geyser.tag: GasBuildingData()})

    # This method distributes workers to mine minerals and gas
    # Fills up the gas first, then the minerals
    def distribute_workers(self) -> None:
        def try_assign_to_gas(worker: Unit) -> bool:
            for geyser in self.bot.gas_buildings.filter(lambda g: g.build_progress == 1).sorted(lambda g: g.distance_to(worker)):
                geyser_data: GasBuildingData = self.gas_data[geyser.tag]
                if geyser_data.current_harvesters < geyser.ideal_harvesters:
                    self.assign_worker(worker.tag, WorkerRole.GAS, geyser.tag)
                    worker(AbilityId.SMART, geyser, queue=True)
                    return True
            return False

        def try_assign_to_minerals(worker: Unit) -> bool:
            for th in self.bot.townhalls.filter(lambda t: t.build_progress == 1).sorted(lambda t: t.distance_to(worker)):
                th_data: TownhallData = self.th_data[th.tag]
                if th_data.current_harvesters < th.ideal_harvesters:
                    self.assign_worker(worker.tag, WorkerRole.MINERALS, th.tag)
                    # TODO: change the random selection when we have a better way to select the mineral field
                    worker(AbilityId.SMART, self.bot.mineral_field.closer_than(10, th).random, queue=True)
                    return True
            return False

        def assign_idle_worker(worker: Unit) -> None:
            worker(AbilityId.STOP_STOP)
            self.assign_worker(worker.tag, WorkerRole.IDLE, None)

        def reallocate_workers(structures: Units, data_dict: dict[int, Union[TownhallData, GasBuildingData]]) -> None:
            for structure in structures:
                structure_data: Union[TownhallData, GasBuildingData] = data_dict[structure.tag]
                if structure_data.current_harvesters > structure.ideal_harvesters:
                    workers: Units = self.bot.workers.filter(lambda w: self.worker_data[w.tag].assigned_to_tag == structure.tag)
                    count: int = 0
                    for worker in workers:
                        count += 1
                        if count > structure.ideal_harvesters:
                            assign_idle_worker(worker)
                            break

        for worker in self.bot.workers:
            if worker.tag not in self.worker_data:
                worker(AbilityId.STOP_STOP)
                self.worker_data.update({worker.tag: WorkerData(WorkerRole.IDLE)})

            data: WorkerData = self.worker_data[worker.tag]

            # If the worker is idle
            # If there is a gas geyser that is not full, assign worker to it
            # Otherwise, assign worker to mine minerals
            if data.role == WorkerRole.IDLE:
                has_been_assigned: bool = try_assign_to_gas(worker)

                if has_been_assigned:
                    continue

                has_been_assigned = try_assign_to_minerals(worker)

                # TODO: this will have to change
                if not has_been_assigned:
                    worker(AbilityId.STOP_STOP)
                    self.assign_worker(worker.tag, WorkerRole.IDLE, None)

            # If there is a worker that has a role but is idle, assign them based on their role
            if data.role == WorkerRole.MINERALS and worker.is_idle:
                try_assign_to_minerals(worker)

            if data.role == WorkerRole.GAS and worker.is_idle:
                try_assign_to_gas(worker)

        # TODO: maybe we dont need to do this every frame
        # If there are too many workers mining a gas geyser, try to reallocate them
        reallocate_workers(self.bot.gas_buildings.filter(lambda g: g.build_progress == 1), self.gas_data)

        # If there are too many workers assigned to a base, try to reallocate them
        reallocate_workers(self.bot.townhalls.filter(lambda t: t.build_progress == 1), self.th_data)

    # This method makes workers mine minerals faster
    def speed_mine(self) -> None:
        mineral_workers: Units = self.find_mineral_workers()
        for worker in mineral_workers:
            self.speed_mine_worker(worker)

    def speed_mine_worker(self, worker: Unit) -> None:
        def move_worker_to_target(worker_to_move: Unit, target_unit: Unit, target_to_move_to: Point2) -> None:
            if 0.75 < worker_to_move.distance_to(target_to_move_to) < 2:
                worker_to_move.move(target_to_move_to)
                worker_to_move(AbilityId.SMART, target_unit, queue=True)
                return

        data: WorkerData = self.worker_data[worker.tag]
        if data.assigned_to_tag is None:
            return
        th: Unit = self.bot.townhalls.by_tag(data.assigned_to_tag)
        if not th:
            return

        if len(worker.orders) == 0:
            # TODO: change the random selection when we have a better way to select the mineral field
            mf: Unit = self.bot.mineral_field.closer_than(10, th).random
            if not self.mineral_targets.__contains__(mf.tag):
                self.mineral_targets.update({mf.tag: mf.position.towards(th, MINING_RADIUS)})
            target: Point2 = self.mineral_targets[mf.tag]
            worker.move(target)
            worker(AbilityId.SMART, mf, queue=True)
            return

        if worker.is_returning and len(worker.orders) == 1:
            target: Point2 = th.position.towards(worker.position, th.radius + worker.radius)
            move_worker_to_target(worker, th, target)

        if not worker.is_returning and len(worker.orders) == 1 and isinstance(worker.order_target, int):
            mf: Unit = self.bot.mineral_field.by_tag(worker.order_target)
            if mf is not None and mf.is_mineral_field:
                if not self.mineral_targets.__contains__(mf.tag):
                    self.mineral_targets.update({mf.tag: mf.position.towards(th, MINING_RADIUS)})
                target = self.mineral_targets[mf.tag]
                move_worker_to_target(worker, mf, target)

    # This method selects all the workers that should be mining minerals
    def find_mineral_workers(self) -> Units:
        def mineral_filter(worker: Unit) -> bool:
            if worker.tag not in self.worker_data:
                return False
            data: WorkerData = self.worker_data[worker.tag]
            return data.role == WorkerRole.MINERALS
        return self.bot.workers.filter(mineral_filter)

    # This method assigns a role to a worker
    def assign_worker(self, worker_tag: int, role: WorkerRole, assigned_to_tag: Optional[int]) -> None:
        # remove worker from previous assignment
        if self.worker_data[worker_tag].assigned_to_tag is not None:
            old_assigned_to_tag = self.worker_data[worker_tag].assigned_to_tag
            if old_assigned_to_tag in self.th_data:
                self.th_data[old_assigned_to_tag].current_harvesters -= 1
            if old_assigned_to_tag in self.gas_data:
                self.gas_data[old_assigned_to_tag].current_harvesters -= 1
        if assigned_to_tag is not None:
            if assigned_to_tag in self.th_data:
                self.th_data[assigned_to_tag].current_harvesters += 1
            if assigned_to_tag in self.gas_data:
                self.gas_data[assigned_to_tag].current_harvesters += 1
        self.worker_data[worker_tag].assign_to(assigned_to_tag, role)

    # This method selects a worker to do a task
    # It selects the closest worker to the location
    # The method takes into account the worker's role
    def select_worker(self, location: Point2, role: WorkerRole) -> Optional[Unit]:
        role_weights = {
            WorkerRole.IDLE: 1,
            WorkerRole.MINERALS: 2,
            WorkerRole.GAS: 3,
            WorkerRole.BUILD: 1
        }

        closest_worker: Optional[Unit] = None
        closest_distance: float = 999999

        for worker in self.bot.workers:
            data: WorkerData = self.worker_data[worker.tag]
            additional_weight: float = 2 if worker.is_carrying_resource else 1
            weighted_distance: float = worker.distance_to(location) * role_weights[data.role] * additional_weight
            if weighted_distance < closest_distance:
                closest_worker = worker
                closest_distance = weighted_distance

        self.assign_worker(closest_worker.tag, role, None)
        return closest_worker

    # This method calculates the mineral targets for the workers
    # It should be called at the start of the game
    def calculate_mineral_targets(self) -> None:
        # TODO: change this when we have the expansion locations with the mineral fields
        for expansion in self.bot.expansion_locations_list:
            for mineral in self.bot.mineral_field.closer_than(10, expansion):
                self.mineral_targets[mineral.tag] = mineral.position.towards(expansion, MINING_RADIUS)
