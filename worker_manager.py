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
        self.assigned_workers_per_mineral_patch: dict[int, int] = {}


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
            for mineral_patch in bot.mineral_field.closer_than(10, townhall):
                self.townhall_data[townhall.tag].assigned_workers_per_mineral_patch.update({mineral_patch.tag: 0})
        for gas_building in bot.gas_buildings:
            self.gas_building_data.update({gas_building.tag: GasBuildingData()})
        for worker in bot.workers:
            self.worker_data.update({worker.tag: WorkerData(WorkerRole.IDLE)})

    def distribute_idle_workers(self, ratio: float) -> None:
        for worker in self.bot.workers:
            worker_data = self.worker_data[worker.tag]
            if worker_data.role == WorkerRole.IDLE:
                # Check if we need more workers on minerals or gas
                # Change this so it makes sense, we should be able to distribute workers based on a ratio of minerals to gas
                # Maybe find the number of workers on minerals and gas and compare that to the ratio
                # Also check if there is a base that needs workers
                # If there is a base that needs workers, send a worker to that base
                # Check if gas is full, if it is, send a worker to minerals
                # Check how many workers are on each patch, assign a worker to the patch with the least workers
                if 1 > ratio:
                    # select closes townhall that has empty capacity
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
                    # select the mineral field that has the least workers, that are close to the townhall
                    # this is too complicated and barely works
                    mf_tag: int | None = None
                    mf: Unit | None = None
                    least_assigned_mf_worker_count: int = 100
                    mineral_fields_in_order_of_distance = sorted(
                        self.bot.mineral_field.filter(lambda mf: mf.distance_to(th) < 10),
                        key=lambda mf: mf.distance_to(th),
                    )

                    for mineral_field in mineral_fields_in_order_of_distance:
                        if self.townhall_data[th_tag].assigned_workers_per_mineral_patch[
                            mineral_field.tag] < least_assigned_mf_worker_count:
                            mf_tag = mineral_field.tag
                            mf = mineral_field
                            least_assigned_mf_worker_count = \
                            self.townhall_data[th_tag].assigned_workers_per_mineral_patch[mineral_field.tag]

                    if th_tag is not None and mf_tag is not None:
                        worker(AbilityId.SMART, mf, True)
                        self.worker_data[worker.tag].change_assignment(mf_tag, WorkerRole.MINERALS)
                        self.townhall_data[th_tag].assigned_workers += 1
                        self.townhall_data[th_tag].assigned_workers_per_mineral_patch[mf_tag] += 1

                else:
                    target = self.bot.vespene_geyser.closest_to(self.bot.start_location)
                    worker(AbilityId.SMART, target, True)
                    self.worker_data[worker.tag].change_assignment(target.tag, WorkerRole.GAS)

    def find_mineral_workers(self) -> Units:
        def mineral_worker_filter(unit: Unit) -> bool:
            worker_data = self.worker_data[unit.tag]
            return worker_data is not None and worker_data.role == WorkerRole.MINERALS

        return self.bot.workers.filter(mineral_worker_filter)

    def speed_mine(self) -> None:
        mineral_workers = self.find_mineral_workers()

        for worker in mineral_workers:
            self.speed_mine_worker(worker)

    def speed_mine_worker(self, worker: Unit) -> None:
        th = self.bot.townhalls.closest_to(worker)

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