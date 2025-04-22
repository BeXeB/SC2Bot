import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class VikingFighterCombat:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot

    def manage_unit(self, unit) -> None:
        nearby_enemies = self.bot.enemy_units.closer_than(9, unit).filter(lambda u: u.is_flying)

        if nearby_enemies.exists:
            target = nearby_enemies.closest_to(unit)
            self.bot.do(unit.attack(target))