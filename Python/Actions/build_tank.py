from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class SiegeTankBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    def build_tank(self):
        available_factory_with_techlab = self.bot.structures.filter(lambda struc: struc.type_id == UnitTypeId.FACTORY and struc.has_techlab)
        if not available_factory_with_techlab:
            print("No available factory with techlab. Therefore no tank is produced")
        if available_factory_with_techlab and self.bot.can_afford(UnitTypeId.SIEGETANK):
            available_factory_with_techlab.random_or(None).train(UnitTypeId.SIEGETANK)
