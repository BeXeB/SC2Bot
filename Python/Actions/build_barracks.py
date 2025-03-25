from __future__ import annotations

import typing
from typing import List

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

from Python.Modules.information_manager import WorkerRole, PlacementType

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class BarracksBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_barracks(self) -> None:
        placement_type = self.bot.information_manager.building_type_to_placement_type[UnitTypeId.BARRACKS]
        build_location = self.bot.map_analyzer.find_placement(placement_type, None)
        can_place = await self.bot.can_place_single(UnitTypeId.BARRACKS, build_location)
        while not can_place:
            print(f"Got invalid Barracks location: {build_location}")
            return
        worker = self.bot.worker_manager.select_worker(build_location, WorkerRole.BUILD)
        if not worker:
            return
        self.bot.busy_workers.update({worker.tag: self.bot.information_manager.build_times[UnitTypeId.BARRACKS]})
        worker.build(UnitTypeId.BARRACKS, build_location)