from __future__ import annotations
import typing

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2
from sc2.unit import Unit
from sc2.units import Units

if typing.TYPE_CHECKING:
    from testbot import MyBot

MINING_RADIUS = 1.325

class WorkerManager:
    bot: MyBot = None

    def __init__(self, bot: MyBot) -> None:
        self.bot = bot

    def distribute_idle_workers(self, ratio: float) -> None:
        for worker in self.bot.workers:
            if worker.is_idle:
                # Check if we need more workers on minerals or gas
                # Change this so it makes sense, we should be able to distribute workers based on a ratio of minerals to gas
                # Maybe find the number of workers on minerals and gas and compare that to the ratio
                # Also check if there is a base that needs workers
                # If there is a base that needs workers, send a worker to that base
                # Check if gas is full, if it is, send a worker to minerals
                if 1 > ratio:
                    target = self.bot.mineral_field.closest_to(self.bot.start_location)
                    worker(AbilityId.SMART, target, True)
                else:
                    worker.gather(self.bot.vespene_geyser.closest_to(self.bot.start_location))

    def find_mineral_workers(self) -> Units:
        def mineral_worker_filter(unit: Unit) -> bool:
            mf = self.bot.mineral_field.find_by_tag(unit.order_target)
            return (unit.is_gathering and mf) or unit.is_carrying_minerals
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

    def select_worker(self, position: Point2) -> Unit | None:
        # Iterate through workers and find the closest worker to the position
        # Take into account the state of the worker
        return None