from sc2.ids.unit_typeid import UnitTypeId

from Python.Modules.information_manager import WorkerRole
import typing
if typing.TYPE_CHECKING:
    from Python.testbot import MyBot


class VespeneBuilder:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        # Track geysers that already have an extractor or are being built
        self.processed_geysers = set()

    async def build_vespene_extractor(self, base_position):
        # Find geysers close to the base position
        geysers = self.bot.vespene_geyser.closer_than(10, base_position)

        for geyser in geysers:
            # Skip geysers that are already processed
            if geyser.tag in self.processed_geysers:
                continue

            # Check if a refinery is already built on the geyser
            if self.__is_vespene_extractor_built(geyser):
                self.processed_geysers.add(geyser.tag)
                continue

            # Build refinery if can_afford
            if self.bot.can_afford(UnitTypeId.REFINERY):
                worker = self.bot.worker_manager.select_worker(geyser.position, WorkerRole.BUILD)
                worker.build(UnitTypeId.REFINERY, geyser, queue=True)
                # self.bot.busy_workers.update({worker.tag: self.bot.REFINERY_BUILD_TIME_STEPS + self.bot.REFINERY_TRAVEL_TIME_STEPS})
                self.bot.busy_workers.update({worker.tag: self.bot.information_manager.build_times[UnitTypeId.REFINERY]})
                # Mark this geyser as processed
                self.processed_geysers.add(geyser.tag)
                return

    def __is_vespene_extractor_built(self, geyser):
        for structure in self.bot.structures:
            if structure.position == geyser.position and structure.type_id == UnitTypeId.REFINERY:
                return True
        return False
