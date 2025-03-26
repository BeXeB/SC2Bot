from __future__ import annotations

import math
import typing
from typing import List

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2

from Python.Modules.information_manager import WorkerRole, PlacementType


class SupplyBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_supply(self):
        placement_type = self.bot.information_manager.building_type_to_placement_type[UnitTypeId.SUPPLYDEPOT]
        build_location = await self.bot.map_analyzer.find_placement(placement_type, UnitTypeId.SUPPLYDEPOT)
        if not build_location:
            print("No location found for Supply depot")
            return
        worker = self.bot.worker_manager.select_worker(build_location, WorkerRole.BUILD)
        if not worker:
            return
        self.bot.busy_workers.update({worker.tag: self.bot.information_manager.build_times[UnitTypeId.SUPPLYDEPOT]})
        worker.build(UnitTypeId.SUPPLYDEPOT, build_location)