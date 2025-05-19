import typing

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.unit import Unit

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot




class VikingFighterCombat:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot

    def manage_unit(self, unit) -> None:
        nearby_enemies = self.bot.enemy_units.closer_than(20, unit)
        nearby_flying_enemies = nearby_enemies.filter(lambda u: u.is_flying)
        nearby_ground_enemies = nearby_enemies.filter(lambda u: not u.is_flying)

        if nearby_flying_enemies.exists:
            self.__enable_fighter_mode(unit)
            target = nearby_flying_enemies.closest_to(unit)
            self.bot.do(unit.attack(target))
        elif nearby_ground_enemies.exists:
            self.__disable_fighter_mode(unit)
            target = nearby_ground_enemies.closest_to(unit)
            self.bot.do(unit.attack(target))
        else:
            self.__disable_fighter_mode(unit)
    def __enable_fighter_mode(self, viking: Unit) -> None:
        if viking.type_id == UnitTypeId.VIKINGASSAULT:
            self.bot.do(viking(AbilityId.MORPH_VIKINGFIGHTERMODE))

    def __disable_fighter_mode(self, viking: Unit):
        print(viking.type_id)
        if viking.type_id == UnitTypeId.VIKINGFIGHTER:
            self.bot.do(viking(AbilityId.MORPH_VIKINGASSAULTMODE))
