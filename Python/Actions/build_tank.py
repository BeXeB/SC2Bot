from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId
import Python.Actions.build_unit_helper as unit_helper

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class SiegeTankBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.unit_builder = unit_helper.UnitBuilderHelper(bot)

    def build_tank(self):
        self.unit_builder.build_unit(UnitTypeId.FACTORY, UnitTypeId.SIEGETANK)