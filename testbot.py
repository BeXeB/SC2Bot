from sc2.bot_ai import BotAI
from worker_manager import WorkerManager
from sc2.ids.unit_typeid import UnitTypeId

class MyBot(BotAI):
    def __init__(self):
        self.worker_manager = WorkerManager(self)

    def get_units(self, unit_type):
        return self.units.filter(lambda u: u.type_id == unit_type)

    async def on_step(self, iteration: int):
        await self.worker_manager.distribute_workers(0)


