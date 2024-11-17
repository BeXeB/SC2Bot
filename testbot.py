from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId

from Actions.BuildBase import BaseBuilder
from Actions.VespeneExtractor import VespeneBuilder


class MyBot(BotAI):
    # Initializing our bot with the BaseBuilder, such that we don't initialize a new BaseBuilder instance on every step.
    def __init__(self) -> None:
        self.base_builder = BaseBuilder(self)
        self.vespene_builder = VespeneBuilder(self)
        self.completed_bases = set()

    async def on_step(self, iteration: int) -> None:
        print(f"iteration: {iteration}")
        # Temporary code to make workers gather minerals
        for worker in self.workers.idle:
            closest_minerals = self.mineral_field.closest_to(worker)
            worker.gather(closest_minerals)

        # Bot code to automatically find next base location, and build a command center, if possible/affordable.
        # We'll have to update this later for the monte carlo tree search, but it will not be difficult whatsoever
        if self.can_afford(UnitTypeId.COMMANDCENTER) and not self.already_pending(UnitTypeId.COMMANDCENTER):
            new_base_location = await self.base_builder.find_next_base_location()
            if new_base_location:
                worker = self.workers.closest_to(new_base_location)
                worker.build(UnitTypeId.COMMANDCENTER, new_base_location)

        # Handle vespene extractor building for each completed base
        for townhall in self.townhalls:
            if townhall.tag not in self.completed_bases and townhall.is_ready:
                await self.vespene_builder.build_vespene_extractor(townhall.position)
                self.completed_bases.add(townhall.tag)






class PeacefulBot(BotAI):
    async def on_step(self, iteration: int) -> None:
        # Gather minerals
        for worker in self.workers.idle:
            closest_minerals = self.mineral_field.closest_to(worker)
            worker.gather(closest_minerals)

        # Build a few units for defense if needed, but don't attack
        if self.can_afford(UnitTypeId.SCV) and not self.already_pending(UnitTypeId.SCV):
            self.train(UnitTypeId.SCV)