from __future__ import annotations

from typing import Optional

from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2
import typing

if typing.TYPE_CHECKING:
    from testbot import MyBot


class WorkerBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_worker(self, close_to: Optional[Point2] = None) -> None:
        # If no position is given, we will use the position of a random townhall, that is ready and has no queue
        available_townhalls = self.bot.townhalls.ready.filter(lambda t: len(t.orders) == 0)
        if len(available_townhalls) == 0:
            return  # No townhall is ready
        if close_to is None:
            closest_th = available_townhalls.random
        else:
            closest_th = available_townhalls.closest_to(close_to)
        self.bot.do(closest_th.train(UnitTypeId.SCV))