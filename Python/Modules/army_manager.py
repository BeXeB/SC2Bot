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
            closest_enemy_building = self.bot.enemy_structures.closest_to(self.bot.start_location)
            for marine in self.bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE):
                marine.attack(closest_enemy_building.position)