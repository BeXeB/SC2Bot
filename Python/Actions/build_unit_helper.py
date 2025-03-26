import typing
from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class UnitBuilderHelper:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    def build_unit(self, unit_builder_structure: UnitTypeId, unit_type: UnitTypeId) -> None:
        available_structure_with_techlab = self.bot.structures.filter(lambda struc: struc.type_id == unit_builder_structure and struc.has_techlab)
        if not available_structure_with_techlab:
            print("No available " + str(unit_builder_structure) + " with techlab. Therefore no " + str(unit_type) + " is produced")
        if available_structure_with_techlab and self.bot.can_afford(unit_type):
            available_structure_with_techlab.random_or(None).train(unit_type)