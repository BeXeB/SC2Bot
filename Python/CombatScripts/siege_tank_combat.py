import typing

from sc2.units import Units

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.unit import Unit

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class SiegeTankCombat:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot

    def manage_unit(self, unit:Unit) -> None:
        nearby_enemies:Units = self.bot.enemy_units.closer_than(15, unit)

        if unit.type_id == UnitTypeId.SIEGETANKSIEGED and nearby_enemies.exists:
            target = nearby_enemies.closest_to(unit)
            self.bot.do(unit.attack(target))

        if self.bot.enemy_units.closer_than(4, unit).exists:
            self.__disable_siege_mode(unit)
        elif self.bot.enemy_units.closer_than(14, unit).exists:
            self.__enable_siege_mode(unit)
        else:
            self.__disable_siege_mode(unit)


    def __enable_siege_mode(self, tank: Unit) -> None:
        if tank.type_id == UnitTypeId.SIEGETANK:
            self.bot.do(tank(AbilityId.SIEGEMODE_SIEGEMODE))

    def __disable_siege_mode(self, tank: Unit) -> None:
        if tank.type_id == UnitTypeId.SIEGETANKSIEGED:
            self.bot.do(tank(AbilityId.UNSIEGE_UNSIEGE))
