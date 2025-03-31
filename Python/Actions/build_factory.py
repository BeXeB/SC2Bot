from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class FactoryBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_factory(self) -> None:
        await self.bot.structure_builder_helper.build_structure(UnitTypeId.FACTORY)

    async def build_tech_lab(self) -> None:
        await self.bot.structure_builder_helper.build_tech_lab(UnitTypeId.FACTORY, UnitTypeId.FACTORYTECHLAB)