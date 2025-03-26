from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class VikingFighterBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    def build_viking(self) -> None:
        self.bot.unit_builder_helper.build_unit(UnitTypeId.STARPORT, UnitTypeId.VIKING)