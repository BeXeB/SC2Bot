import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class ArmyManager:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot

    # TODO: Analyze the map to determine where to attack
    # TODO: Implement squads

    def manage_army(self):
        marine_count = self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE).amount
        if marine_count > 20:
            position = self.bot.enemy_start_locations[0] if self.bot.enemy_structures.empty else self.bot.enemy_structures.closest_to(self.bot.start_location).position
            for marine in self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE):
                marine.attack(position)