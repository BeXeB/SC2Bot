import math
import typing
from typing import Optional, Union, Tuple

from sc2.ids.ability_id import AbilityId
from sc2.position import Point2
from sc2.unit import Unit, UnitOrder
from sc2.units import Units

from Python.Modules.information_manager import (WorkerRole, WorkerData, GasBuildingData, TownhallData,
                                                StructureData, UnitData)

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

MINING_RADIUS = 1.325


class WorkerManager:
    bot: 'MyBot'
    mineral_targets: dict[int, Point2]

    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot
        self.mineral_targets = {}
        self.__calculate_mineral_targets()

    # This method distributes workers to mine minerals and gas
    # Fills up the gas first, then the minerals
    def distribute_workers(self) -> None:
        def __try_assign_to_gas(worker: Unit) -> bool:
            for geyser in self.bot.gas_buildings.filter(lambda g: g.build_progress == 1).sorted(
                    lambda g: g.distance_to(worker)):
                geyser_data: GasBuildingData = self.bot.information_manager.gas_data[geyser.tag]
                if geyser_data.current_harvesters < geyser.ideal_harvesters:
                    self.assign_worker(worker.tag, WorkerRole.GAS, geyser.tag)
                    worker(AbilityId.SMART, geyser, queue=True)
                    return True
            return False

        def __try_assign_to_minerals(worker: Unit) -> bool:
            for townhall in self.bot.townhalls.filter(lambda t: t.build_progress == 1).sorted(
                    lambda t: t.distance_to(worker)):
                townhall_data: TownhallData = self.bot.information_manager.townhall_data[townhall.tag]
                if townhall_data is None:
                    continue
                if townhall_data.current_harvesters < townhall.ideal_harvesters:
                    self.assign_worker(worker.tag, WorkerRole.MINERALS, townhall.tag)
                    fields = self.bot.mineral_field.closer_than(10, townhall)
                    if fields.amount > 0:
                        mineral_field = fields.random
                        if mineral_field is not None:
                            worker(AbilityId.SMART, mineral_field, queue=True)
                        else:
                            self.assign_worker(worker.tag, WorkerRole.IDLE, None)
                        return True
            return False

        def __assign_idle_worker(worker: Unit) -> None:
            worker(AbilityId.STOP_STOP)
            self.assign_worker(worker.tag, WorkerRole.IDLE, None)

        def __reallocate_workers(structures: Units, data_dict: dict[int, Union[TownhallData, GasBuildingData]]) -> None:
            for structure in structures:
                structure_data: Union[TownhallData, GasBuildingData] = data_dict[structure.tag]
                if structure_data.current_harvesters > structure.ideal_harvesters:
                    workers: Units = self.bot.workers.filter(
                        lambda w: self.bot.information_manager.worker_data[w.tag].assigned_to_tag == structure.tag)
                    count: int = 0
                    for worker in workers:
                        count += 1
                        if count > structure.ideal_harvesters:
                            __assign_idle_worker(worker)
                            break

        def __assign_to_gas_if_needed(worker: Unit) -> None:
            for gas in self.bot.gas_buildings.ready:
                gas_data = self.bot.information_manager.gas_data[gas.tag]
                if gas_data.current_harvesters >= gas.ideal_harvesters:
                    continue
                self.assign_worker(worker.tag, WorkerRole.GAS, gas_data.tag)
                worker(AbilityId.SMART, gas, queue=False)
                return

        def __fix_afk_workers(data: WorkerData, worker: Unit) -> None:
            # Only happens if the worker is a mineral worker
            if data.role != WorkerRole.MINERALS:
                return
            # Only happens if the worker's order is move
            orders: Tuple[UnitOrder, ...] = worker.orders
            if len(orders) == 0 or orders[0].ability.id != AbilityId.MOVE:
                return
            prev_unit: Optional[Unit] = self.bot._units_previous_map.get(data.tag, None)
            # If the worker is newly created, we return
            if prev_unit is None:
                return
            prev_loc: Point2 = prev_unit.position
            # If the worker is moving, we return
            if worker.position != prev_loc:
                return
            # The worker is not moving, so we stop it and assign it to idle
            worker(AbilityId.STOP_STOP)
            self.assign_worker(worker.tag, WorkerRole.IDLE, None)

        for worker in self.bot.workers:
            data: WorkerData = self.bot.information_manager.worker_data[worker.tag]

            # If the worker is idle
            # If there is a gas geyser that is not full, assign worker to it
            # Otherwise, assign worker to mine minerals
            if data.role == WorkerRole.IDLE:
                has_been_assigned: bool = __try_assign_to_gas(worker)

                if has_been_assigned:
                    continue

                has_been_assigned = __try_assign_to_minerals(worker)

                if not has_been_assigned:
                    worker(AbilityId.STOP_STOP)
                    self.assign_worker(worker.tag, WorkerRole.IDLE, None)

            # If there is a worker that has a role but is idle, assign them based on their role
            if data.role == WorkerRole.MINERALS and worker.is_idle:
                __try_assign_to_minerals(worker)

            if data.role == WorkerRole.GAS and worker.is_idle:
                __try_assign_to_gas(worker)

            if data.role == WorkerRole.MINERALS:
                __assign_to_gas_if_needed(worker)

            __fix_afk_workers(data, worker)

        # If there are too many workers mining a gas geyser, try to reallocate them
        __reallocate_workers(self.bot.gas_buildings.filter(lambda g: g.build_progress == 1),
                             self.bot.information_manager.gas_data)

        # If there are too many workers assigned to a base, try to reallocate them
        __reallocate_workers(self.bot.townhalls.filter(lambda t: t.build_progress == 1),
                             self.bot.information_manager.townhall_data)

    # This method makes workers mine minerals faster
    def speed_mine(self) -> None:
        mineral_workers: Units = self.__find_mineral_workers()
        for worker in mineral_workers:
            self.__speed_mine_worker(worker)

    def __speed_mine_worker(self, worker: Unit) -> None:
        def __move_worker_to_target(worker_to_move: Unit, target_unit: Unit, target_to_move_to: Point2) -> None:
            if 0.75 < worker_to_move.distance_to(target_to_move_to) < 2:
                worker_to_move.move(target_to_move_to)
                worker_to_move(AbilityId.SMART, target_unit, queue=True)
                return

        data: WorkerData = self.bot.information_manager.worker_data[worker.tag]
        if data.assigned_to_tag is None:
            return
        townhall: Unit = self.bot.townhalls.by_tag(data.assigned_to_tag)
        if not townhall:
            return

        if len(worker.orders) == 0:
            mineral_field: Unit = self.bot.mineral_field.closer_than(10, townhall).random
            if not self.mineral_targets.__contains__(mineral_field.tag):
                self.mineral_targets.update(
                    {mineral_field.tag: mineral_field.position.towards(townhall, MINING_RADIUS)})
            target: Point2 = self.mineral_targets[mineral_field.tag]
            worker.move(target)
            worker(AbilityId.SMART, mineral_field, queue=True)
            return

        if worker.is_returning and len(worker.orders) == 1:
            target: Point2 = townhall.position.towards(worker.position, townhall.radius + worker.radius)
            __move_worker_to_target(worker, townhall, target)

        if not worker.is_returning and len(worker.orders) == 1 and isinstance(worker.order_target, int):
            target = worker.order_target
            if self.bot.mineral_field.__contains__(target):
                mineral_field: Unit = self.bot.mineral_field.by_tag(worker.order_target)
                if mineral_field is not None and mineral_field.is_mineral_field:
                    if not self.mineral_targets.__contains__(mineral_field.tag):
                        self.mineral_targets.update(
                            {mineral_field.tag: mineral_field.position.towards(townhall, MINING_RADIUS)})
                    target = self.mineral_targets[mineral_field.tag]
                    __move_worker_to_target(worker, mineral_field, target)

    # This method selects all the workers that should be mining minerals
    def __find_mineral_workers(self) -> Units:
        def __mineral_filter(worker: Unit) -> bool:
            if worker.tag not in self.bot.information_manager.worker_data:
                return False
            data: WorkerData = self.bot.information_manager.worker_data[worker.tag]
            return data.role == WorkerRole.MINERALS

        return self.bot.workers.filter(__mineral_filter)

    # This method assigns a role to a worker
    def assign_worker(self, worker_tag: int, role: WorkerRole, assigned_to_tag: Optional[int]) -> None:
        worker_data = self.bot.information_manager.worker_data
        townhall_data = self.bot.information_manager.townhall_data
        gas_data = self.bot.information_manager.gas_data

        # remove worker from previous assignment
        if worker_tag not in worker_data:
            return

        if worker_data[worker_tag].assigned_to_tag is not None:
            old_assigned_to_tag = worker_data[worker_tag].assigned_to_tag
            if old_assigned_to_tag in townhall_data:
                townhall_data[old_assigned_to_tag].current_harvesters -= 1
            if old_assigned_to_tag in gas_data:
                gas_data[old_assigned_to_tag].current_harvesters -= 1
        if assigned_to_tag is not None:
            if assigned_to_tag in townhall_data:
                townhall_data[assigned_to_tag].current_harvesters += 1
            if assigned_to_tag in gas_data:
                gas_data[assigned_to_tag].current_harvesters += 1
        worker_data[worker_tag].assign_to(assigned_to_tag, role)

    # This method selects a worker to do a task
    # It selects the closest worker to the location
    # The method takes into account the worker's role
    def select_worker(self, location: Point2, role: WorkerRole) -> Optional[Unit]:
        role_weights = {
            WorkerRole.IDLE: 1,
            WorkerRole.MINERALS: 2,
            WorkerRole.GAS: 3,
            WorkerRole.SCOUT: 10000,
            WorkerRole.BUILD: math.inf
        }

        closest_worker: Optional[Unit] = None
        closest_distance: float = math.inf

        for worker in self.bot.workers:
            data: WorkerData = self.bot.information_manager.worker_data[worker.tag]
            weighted_distance: float = worker.distance_to(location) * role_weights[data.role]
            if weighted_distance < closest_distance:
                closest_worker = worker
                closest_distance = weighted_distance

        if closest_worker:
            self.assign_worker(closest_worker.tag, role, None)

        return closest_worker

    # This method calculates the mineral targets for the workers
    # It should be called at the start of the game
    def __calculate_mineral_targets(self) -> None:
        for expansion in self.bot.expansion_locations_list:
            for mineral in self.bot.mineral_field.closer_than(10, expansion):
                self.mineral_targets[mineral.tag] = mineral.position.towards(expansion, MINING_RADIUS)
