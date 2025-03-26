import typing
from sc2.ids.unit_typeid import UnitTypeId
from Python.Modules.information_manager import WorkerRole
if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class StructureBuilderHelper:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    async def build_structure(self, structure_type: UnitTypeId) -> None:
        placement_type = self.bot.information_manager.building_type_to_placement_type[structure_type]
        build_location = await self.bot.map_analyzer.find_placement(placement_type, structure_type)
        if not build_location:
            print("No location found for " + str(structure_type))
            return
        worker = self.bot.worker_manager.select_worker(build_location, WorkerRole.BUILD)
        if not worker:
            return
        self.bot.busy_workers.update({worker.tag: self.bot.information_manager.build_times[structure_type]})
        worker.build(structure_type, build_location)


