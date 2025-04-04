﻿import typing

from sc2.ids.unit_typeid import UnitTypeId
from sc2.units import Units

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class ArmyManager:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot
        self.unit_exclusion_list = self.bot.information_manager.units_to_ignore_for_army

    # TODO: Analyze the map to determine where to attack
    # TODO: Implement squads

    def manage_army(self) -> None:
        marine_count = self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE).amount
        if marine_count > 20:
            position = self.bot.enemy_start_locations[0] if self.bot.enemy_structures.empty else self.bot.enemy_structures.closest_to(self.bot.start_location).position
            for marine in self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE):
                marine.attack(position)


    def check_base_radius(self) -> None:
        for building in self.bot.structures:
            if any (self.bot.enemy_units.closer_than(10, building)):
                position = building.position
                combat_units = self.__units_to_include()
                for unit in combat_units:
                    unit.attack(position)
                break

    def __units_to_include(self) -> Units:
        return self.bot.units.exclude_type(self.unit_exclusion_list)