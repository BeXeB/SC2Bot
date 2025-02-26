from __future__ import annotations

import math
import typing
from typing import Dict

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

from Python.Modules.information_manager import WorkerRole


class SupplyBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        left_or_right = -1 if self.bot.start_location.x > self.bot.game_info.map_size.x/2 else 1
        start_location: Point2 = self.bot.start_location
        start_location = (
            Point2((math.floor(start_location.x), math.floor(start_location.y)))) \
            if left_or_right == 1 else (
            Point2((math.ceil(start_location.x), math.ceil(start_location.y))))
        first_supply_position: Point2 = Point2((start_location.x - 9 * left_or_right, start_location.y - 5 * left_or_right))
        self.possible_supply_positions: Dict[Point2, bool] = {}
        for i in range(5):
            for j in range(5):
                self.possible_supply_positions.update({
                    Point2((first_supply_position.x + i * 2 * left_or_right, first_supply_position.y - j * 2 * left_or_right))
                    : False
                })

    async def build_supply(self):
        if self.possible_supply_positions is None:
            return
        for position in self.possible_supply_positions:
            if self.possible_supply_positions[position]:
                continue
            can_place = await self.bot.can_place_single(UnitTypeId.SUPPLYDEPOT, position)
            if not can_place:
                continue
            worker = self.bot.worker_manager.select_worker(position, WorkerRole.BUILD)
            if not worker:
                break
            self.bot.busy_workers.update({worker.tag: self.bot.information_manager.build_times[UnitTypeId.SUPPLYDEPOT]})
            worker.build(UnitTypeId.SUPPLYDEPOT, position)
            self.bot.information_manager.worker_data[worker.tag].orders = worker.orders
            self.possible_supply_positions[position] = True
            break

    async def destroy_supply(self, supply_position: Point2):
        self.possible_supply_positions[supply_position] = False