from sc2.bot_ai import BotAI
from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId

from Actions.build_supply import SupplyBuilder


class MyBot(BotAI):
    async def on_start(self):
        self.supply_builder = SupplyBuilder(self)

    async def on_step(self, iteration: int):
        await self.distribute_workers()

        if self.can_afford(UnitTypeId.SUPPLYDEPOT) and self.supply_cap < 200:
            await self.supply_builder.build_supply()

    async def on_building_construction_complete(self, unit):
        if unit.type_id == UnitTypeId.SUPPLYDEPOT:
            unit(AbilityId.MORPH_SUPPLYDEPOT_LOWER)