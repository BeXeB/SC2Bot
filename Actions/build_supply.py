from __future__ import annotations

from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

from worker_manager import WorkerRole


class SupplyBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        left_or_right = -1 if self.bot.start_location.x > self.bot.game_info.map_size.x/2 else 1
        start_location = self.bot.start_location
        first_supply_position = Point2((start_location.x + 2  * left_or_right, start_location.y - 6))
        self.possible_supply_positions = []
        for i in range(1, 6):
            for j in range(1, 6):
                self.possible_supply_positions.append(
                    Point2((first_supply_position.x + i * 2 * left_or_right, first_supply_position.y + j * 2))
                )

    async def build_supply(self):
        if self.possible_supply_positions:
            for position in self.possible_supply_positions:
                can_place = await self.bot.can_place_single(UnitTypeId.SUPPLYDEPOT, position)
                if can_place:
                    worker = self.bot.worker_manager.select_worker(position, WorkerRole.BUILD)
                    if worker:
                        # self.bot.busy_workers.update({worker.tag: self.bot.SUPPLY_BUILD_TIME_STEPS + self.bot.SUPPLY_TRAVEL_TIME_STEPS})
                        self.bot.busy_workers.update({worker.tag: self.bot.SUPPLY_BUILD_TIME_SECONDS + self.bot.SUPPLY_TRAVEL_TIME_SECONDS})
                        worker.build(UnitTypeId.SUPPLYDEPOT, position)
                        self.possible_supply_positions.remove(position)
                        break