from __future__ import annotations
import typing
from typing import List

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

from Modules.worker_manager import WorkerRole

if typing.TYPE_CHECKING:
    from testbot import MyBot

class BaracksBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.build_locations = self.__find_build_locations()

    # Figure out where the barracks will be built
    def __find_build_locations(self) -> List[Point2]:
        pass

    async def build_baracks(self) -> None:
        if self.build_locations is None:
            return
        for build_location in self.build_locations:
            can_place = self.bot.can_place_single(UnitTypeId.BARRACKS, build_location)
            if not can_place:
                continue
            worker = self.bot.worker_manager.select_worker(build_location, WorkerRole.BUILD)
            if not worker:
                continue
            self.bot.busy_workers.update({worker.tag: self.bot.BARRACKS_BUILD_TIME_SECONDS})
            worker.build(UnitTypeId.BARRACKS, build_location)
            # TODO: Change this so it becomes available if its destroyed
            self.build_locations.remove(build_location)
            break