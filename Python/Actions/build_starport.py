from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId
import Python.Actions.build_structure_helper as structure_helper

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class StarportBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.structure_builder = structure_helper.StructureBuilderHelper(bot)

    async def build_starport(self) -> None:
        await self.structure_builder.build_structure(UnitTypeId.STARPORT)