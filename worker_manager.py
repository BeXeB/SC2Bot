from __future__ import annotations
import typing
from enum import Enum

from sc2.ids.ability_id import AbilityId
from sc2.position import Point2
from sc2.unit import Unit
from sc2.units import Units

if typing.TYPE_CHECKING:
    from testbot import MyBot

MINING_RADIUS = 1.325


class WorkerRole(Enum):
    IDLE = 0
    MINERALS = 1
    GAS = 2
    BUILD = 3
    SCOUT = 4
    REPAIR = 5
    ATTACK = 6


class TownhallData:
    def __init__(self) -> None:
        self.assigned_workers: int = 0


class GasBuildingData:
    def __init__(self) -> None:
        self.assigned_workers: int = 0


class WorkerData:
    def __init__(self, role: WorkerRole) -> None:
        self.role: WorkerRole = role
        self.assigned_to_tag: int | None = None

    def change_assignment(self, tag: int | None, role: WorkerRole) -> None:
        self.assigned_to_tag = tag
        self.role = role


class WorkerManager:
    bot: MyBot = None
    worker_data: dict[int, WorkerData] = {}
    townhall_data: dict[int, TownhallData] = {}
    gas_building_data: dict[int, GasBuildingData] = {}

    def __init__(self, bot: MyBot) -> None:
        self.bot = bot
        for townhall in bot.townhalls:
            self.townhall_data.update({townhall.tag: TownhallData()})
        for gas_building in bot.gas_buildings:
            self.gas_building_data.update({gas_building.tag: GasBuildingData()})
        for worker in bot.workers:
            self.worker_data.update({worker.tag: WorkerData(WorkerRole.IDLE)})

    def add_worker(self, worker: Unit, role: WorkerRole) -> None:
        self.worker_data.update({worker.tag: WorkerData(role)})

    def remove_worker(self, tag: int) -> None:
        worker_data = self.worker_data[tag]
        if worker_data.role == WorkerRole.MINERALS:
            th_tag = worker_data.assigned_to_tag
            self.townhall_data[th_tag].assigned_workers -= 1

        if worker_data.role == WorkerRole.GAS:
            gas_building_tag = worker_data.assigned_to_tag
            self.gas_building_data[gas_building_tag].assigned_workers -= 1

        self.worker_data.pop(tag)

    def add_townhall(self, townhall: Unit) -> None:
        self.townhall_data.update({townhall.tag: TownhallData()})

    def distribute_workers(self) -> None:
        for worker in self.bot.workers:
            worker_data = self.worker_data[worker.tag]
            if worker_data.role == WorkerRole.IDLE:
                # if there is a gas building that needs workers, assign a worker to it
                gas_building_tag: int | None = None
                gas_building: Unit | None = None

                gas_buildings_in_order_of_distance = sorted(
                    self.bot.gas_buildings,
                    key=lambda gb: gb.distance_to(worker),
                )

                for gas_building in gas_buildings_in_order_of_distance:
                    if self.gas_building_data[gas_building.tag].assigned_workers < gas_building.ideal_harvesters:
                        gas_building_tag = gas_building.tag
                        gas_building = gas_building
                        break

                if gas_building is not None:
                    worker(AbilityId.SMART, gas_building, True)
                    self.worker_data[worker.tag].change_assignment(gas_building_tag, WorkerRole.GAS)
                    self.gas_building_data[gas_building_tag].assigned_workers += 1
                    continue

                # if there is a base that needs workers, assign a worker to it
                th_tag: int | None = None
                th: Unit | None = None

                townhalls_in_order_of_distance = sorted(
                    self.bot.townhalls,
                    key=lambda th: th.distance_to(worker),
                )

                for townhall in townhalls_in_order_of_distance:
                    if self.townhall_data[townhall.tag].assigned_workers < townhall.ideal_harvesters:
                        th_tag = townhall.tag
                        th = townhall
                        break

                # select a field randomly
                if th is not None:
                    mfs = self.bot.mineral_field.closer_than(10, th)
                    mf = mfs.random
                    worker(AbilityId.SMART, mf, True)
                    self.worker_data[worker.tag].change_assignment(th_tag, WorkerRole.MINERALS)
                    self.townhall_data[th_tag].assigned_workers += 1
            if worker_data.role == WorkerRole.MINERALS and worker.is_idle:
                # if a mineral worker is idle, send it to mine in the closest townhall
                townhalls_in_order_of_distance = sorted(
                    self.bot.townhalls,
                    key=lambda th: th.distance_to(worker),
                )

                for townhall in townhalls_in_order_of_distance:
                    if self.townhall_data[townhall.tag].assigned_workers < townhall.ideal_harvesters:
                        mfs = self.bot.mineral_field.closer_than(10, townhall)
                        mf = mfs.random
                        worker(AbilityId.SMART, mf, True)
                        self.worker_data[worker.tag].change_assignment(townhall.tag, WorkerRole.MINERALS)
                        self.townhall_data[townhall.tag].assigned_workers += 1
                        break
            if worker_data.role == WorkerRole.GAS and worker.is_idle:
                # if a gas worker is idle, find a gas building to mine
                gas_buildings_in_order_of_distance = sorted(
                    self.bot.gas_buildings,
                    key=lambda gb: gb.distance_to(worker),
                )
                for gas_building in gas_buildings_in_order_of_distance:
                    if self.gas_building_data[gas_building.tag].assigned_workers < gas_building.ideal_harvesters:
                        worker(AbilityId.SMART, gas_building, True)
                        self.worker_data[worker.tag].change_assignment(gas_building.tag, WorkerRole.GAS)
                        self.gas_building_data[gas_building.tag].assigned_workers += 1
                        break

        # if a base has more workers than it needs, reassign them
        for townhall in self.bot.townhalls:
            th_data = self.townhall_data[townhall.tag]
            if th_data.assigned_workers > townhall.ideal_harvesters:
                for worker in self.bot.workers.closer_than(10, townhall):
                    if th_data.assigned_workers <= townhall.ideal_harvesters:
                        break
                    if self.worker_data[worker.tag].role == WorkerRole.MINERALS and self.worker_data[worker.tag].assigned_to_tag == townhall.tag:
                        townhalls_in_order_of_distance = sorted(
                            self.bot.townhalls,
                            key=lambda th: th.distance_to(worker),
                        )
                        for other_townhall in townhalls_in_order_of_distance:
                            if self.townhall_data[other_townhall.tag].assigned_workers < other_townhall.ideal_harvesters:
                                mfs = self.bot.mineral_field.closer_than(10, other_townhall)
                                mf = mfs.random
                                worker(AbilityId.SMART, mf, True)
                                self.worker_data[worker.tag].change_assignment(other_townhall.tag, WorkerRole.MINERALS)
                                self.townhall_data[other_townhall.tag].assigned_workers += 1
                                th_data.assigned_workers -= 1
                                break

    def find_mineral_workers(self) -> Units:
        def mineral_worker_filter(unit: Unit) -> bool:
            if unit.tag not in self.worker_data:
                return False
            worker_data = self.worker_data[unit.tag]
            return worker_data is not None and worker_data.role == WorkerRole.MINERALS

        return self.bot.workers.filter(mineral_worker_filter)

    def speed_mine(self) -> None:
        mineral_workers = self.find_mineral_workers()

        for worker in mineral_workers:
            self.speed_mine_worker(worker)

    def speed_mine_worker(self, worker: Unit) -> None:
        worker_data = self.worker_data[worker.tag]
        if worker_data.assigned_to_tag is None:
            return None
        th = self.bot.townhalls.by_tag(worker_data.assigned_to_tag)

        if len(worker.orders) == 0:
            mf = self.bot.mineral_field.closest_to(th)
            target: Point2 = mf.position
            target = target.towards(th.position, MINING_RADIUS)
            worker.move(target)
            worker(AbilityId.SMART, mf, True)
            return None

        if worker.is_returning and len(worker.orders) == 1:
            target: Point2 = th.position
            target = target.towards(worker.position, th.radius + worker.radius)
            if 0.75 < worker.distance_to(target) < 2:
                worker.move(target)
                worker(AbilityId.SMART, th, True)
                return None

        if not worker.is_returning and len(worker.orders) == 1 and isinstance(worker.order_target, int):
            mf = self.bot.mineral_field.find_by_tag(worker.order_target)
            if mf is not None and mf.is_mineral_field:
                target = mf.position
                target = target.towards(th.position, MINING_RADIUS)
                if 0.75 < worker.distance_to(target) < 2:
                    worker.move(target)
                    worker(AbilityId.SMART, mf, True)
                    return None

    def select_worker(self, position: Point2, selected_role: WorkerRole) -> Unit | None:
        role_weights = {
            WorkerRole.BUILD: 0.1,
            WorkerRole.IDLE: 0.5,
            WorkerRole.MINERALS: 1,
            WorkerRole.GAS: 2,
            WorkerRole.SCOUT: 2,
            WorkerRole.REPAIR: 5,
            WorkerRole.ATTACK: 5,
        }
        closest_worker: Unit | None = None
        closest_distance: float = 100000

        for worker in self.bot.workers:
            additional_weight = 2 if worker.is_returning else 1
            worker_data = self.worker_data[worker.tag]
            distance = worker.distance_to(position)
            if distance * role_weights[worker_data.role] * additional_weight < closest_distance:
                closest_distance = distance
                closest_worker = worker

        self.worker_data[closest_worker.tag].change_assignment(None, selected_role)
        return closest_worker