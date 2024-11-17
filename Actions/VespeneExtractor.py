from sc2.ids.unit_typeid import UnitTypeId
from sc2.units import Units


class VespeneBuilder:
    def __init__(self, bot):
        self.bot = bot
        self.processed_geysers = set()  # Track geysers that already have an extractor or are being built

    async def build_vespene_extractor(self, base_position):
        # Find geysers close to the base position
        geysers = self.bot.vespene_geyser.closer_than(10, base_position)  # Adjust range if needed

        for geyser in geysers:
            # Skip geysers that are already processed
            if geyser.tag in self.processed_geysers:
                continue

            # Check if a refinery is already built on the geyser
            if self.is_vespene_extractor_built(geyser):
                self.processed_geysers.add(geyser.tag)
                continue

            # Build refinery if affordable
            if self.bot.can_afford(UnitTypeId.REFINERY):
                worker = self.bot.workers.closest_to(geyser)
                worker.build(UnitTypeId.REFINERY, geyser)
                self.processed_geysers.add(geyser.tag)  # Mark this geyser as processed
                return

    def is_vespene_extractor_built(self, geyser):
        for structure in self.bot.structures:
            if structure.position == geyser.position and structure.type_id == UnitTypeId.REFINERY:
                return True
        return False
