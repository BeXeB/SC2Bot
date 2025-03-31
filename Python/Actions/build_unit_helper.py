import typing
from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class UnitBuilderHelper:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_non_techlab_unit(self, unit_builder_structure: UnitTypeId, unit_type: UnitTypeId):
        if self.bot.can_afford(unit_type):
            available_structure = self.bot.structures.filter(lambda struc: struc.type_id == unit_builder_structure)
            available_structure.random_or(None).train(unit_type)

    async def build_techlab_unit(self, unit_builder_structure: UnitTypeId, unit_type: UnitTypeId) -> None:
        available_structure_with_techlab = self.bot.structures.filter(lambda struc: struc.type_id == unit_builder_structure and struc.has_techlab)
        if not available_structure_with_techlab:
            print("No available " + str(unit_builder_structure) + " with techlab. Therefore no " + str(unit_type) + " is produced")
            return None
        if self.bot.can_afford(unit_type):
            available_structure_with_techlab.random_or(None).train(unit_type)