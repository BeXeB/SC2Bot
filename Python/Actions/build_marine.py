from __future__ import annotations

from typing import Optional

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2
import typing

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot


class MarineBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_marine(self, close_to: Optional[Point2] = None) -> None:
        # If no position is given, we will use the position of a random townhall, that is ready and has no queue
        available_barracks = self.bot.structures.filter(lambda sr: sr.type_id == UnitTypeId.BARRACKS).ready.filter(lambda t: len(t.orders) == 0)
        if len(available_barracks) == 0:
            return  # No townhall is ready
        if close_to is None:
            closest_barracks = available_barracks.random
        else:
            closest_barracks = available_barracks.closest_to(close_to)
        self.bot.do(closest_barracks.train(UnitTypeId.MARINE))