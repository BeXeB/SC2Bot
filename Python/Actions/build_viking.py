from __future__ import annotations

import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class VikingFighterBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    def build_viking(self):
        available_starport_with_techlab = self.bot.structures.filter(lambda struc: struc.type_id == UnitTypeId.STARPORT and struc.has_techlab)
        if not available_starport_with_techlab:
            print("No available starport with techlab. Therefore no viking is produced")
        if available_starport_with_techlab and self.bot.can_afford(UnitTypeId.VIKING):
            available_starport_with_techlab.random_or(None).train(UnitTypeId.VIKING)
