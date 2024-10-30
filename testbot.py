from sc2.bot_ai import BotAI
from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.unit import Unit
from sc2.units import Units
from worker_manager import WorkerManager

class MyBot(BotAI):
    worker_manager = None

    def get_units(self, unit_type: UnitTypeId) -> Units:
        return self.units.filter(lambda u: u.type_id == unit_type)

    def get_unit_by_tag(self, tag: int) -> Unit:
        return self.units.find_by_tag(tag)

    async def on_start(self) -> None:
        self.worker_manager = WorkerManager(self)

    async def on_step(self, iteration: int) -> None:
        self.worker_manager.speed_mine()