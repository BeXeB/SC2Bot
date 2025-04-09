import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class VikingFighterCombat:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    def check_for_enemies_in_range(self) -> None:
        viking_units = self.bot.units.filter(lambda u: u.type_id == UnitTypeId.VIKINGFIGHTER)

        for unit in viking_units:
            nearby_enemies = self.bot.enemy_units.closer_than(9, unit)

            if nearby_enemies.exists:
                target = nearby_enemies.closest_to(unit)
                self.bot.do(unit.attack(target))