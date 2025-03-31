from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class BarracksBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_barracks(self) -> None:
        await self.bot.structure_builder_helper.build_structure(UnitTypeId.BARRACKS)

    def build_tech_lab(self) -> None:
        self.bot.structure_builder_helper.build_tech_lab(UnitTypeId.BARRACKS, UnitTypeId.BARRACKSTECHLAB)