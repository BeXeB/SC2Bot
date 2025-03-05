from __future__ import annotations

import typing
from typing import List

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

from Python.Modules.information_manager import WorkerRole

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class BarracksBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.build_locations = self.__find_build_locations()

    # Figure out where the barracks will be built
    def __find_build_locations(self) -> List[Point2]:
        left_or_right = -1 if self.bot.start_location.x > self.bot.game_info.map_size.x / 2 else 1
        start_location = self.bot.start_location
        first_barracks_position = Point2((start_location.x + (6+(-1*left_or_right)) * left_or_right, start_location.y + 10 * left_or_right))
        possible_barracks_positions = []
        for i in range(2):
            for j in range(6):
                if i == 0 and j == 1:
                    continue
                possible_barracks_positions.append(
                    Point2((first_barracks_position.x + i * 6 * left_or_right, first_barracks_position.y - j * 3 * left_or_right))
                )
        return possible_barracks_positions

    async def build_barracks(self) -> None:
        if self.build_locations is None:
            return
        for build_location in self.build_locations:
            can_place = await self.bot.can_place_single(UnitTypeId.BARRACKS, build_location)
            if not can_place:
                continue
            worker = self.bot.worker_manager.select_worker(build_location, WorkerRole.BUILD)
            if not worker:
                break
            self.bot.busy_workers.update({worker.tag: self.bot.information_manager.build_times[UnitTypeId.BARRACKS]})
            worker.build(UnitTypeId.BARRACKS, build_location)
            break