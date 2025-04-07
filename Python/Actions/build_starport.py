from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class StarportBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_starport(self) -> None:
        await self.bot.structure_builder_helper.build_structure(UnitTypeId.STARPORT)

    async def build_tech_lab(self) -> None:
        await self.bot.structure_builder_helper.build_tech_lab(UnitTypeId.STARPORT, UnitTypeId.STARPORTTECHLAB)