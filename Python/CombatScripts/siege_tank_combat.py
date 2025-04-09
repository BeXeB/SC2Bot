import typing

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.unit import Unit

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class SiegeTankCombat:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    def check_for_enemies_in_range(self) -> None:
        siege_units = self.bot.units.filter(lambda u: u.type_id == UnitTypeId.SIEGETANK or u.type_id == UnitTypeId.SIEGETANKSIEGED)

        for unit in siege_units:
            nearby_enemies = self.bot.enemy_units.closer_than(15, unit)

            if unit.type_id == UnitTypeId.SIEGETANKSIEGED and nearby_enemies.exists:
                target = nearby_enemies.closest_to(unit)
                self.bot.do(unit.attack(target))
            if self.bot.enemy_units.closer_than(4, unit).exists:
                self.__from_siege_mode(unit)
            if self.bot.enemy_units.closer_than(15, unit).exists:
                self.__to_siege_mode(unit)


    def __to_siege_mode(self, tank: Unit) -> None:
        if tank.type_id == UnitTypeId.SIEGETANK:
            self.bot.do(tank(AbilityId.SIEGEMODE_SIEGEMODE))

    def __from_siege_mode(self, tank: Unit) -> None:
        if tank.type_id == UnitTypeId.SIEGETANKSIEGED:
            self.bot.do(tank(AbilityId.UNSIEGE_UNSIEGE))

# When targets are within range 13 of siegetank, transform to siege mode and attack
# If targets are too close (Range 2) transform to tank mode
    # Make start transformation when enemies are within range 3 or 4 of sieged tank
    # Transform back until none are near
# It takes 3.5417 seconds to transform

# Sieging ability = AbilityId.SIEGEMODE_SIEGEMODE
# Unsieging ability = AbilityId.UNSIEGE_UNSIEGE
