from sc2.bot_ai import BotAI
from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2
from sc2.unit import Unit
from sc2.units import Units
from worker_manager import WorkerManager, WorkerRole


class MyBot(BotAI):
    worker_manager = None

    def get_units(self, unit_type: UnitTypeId) -> Units:
        return self.units.filter(lambda u: u.type_id == unit_type)

    def get_unit_by_tag(self, tag: int) -> Unit:
        return self.units.find_by_tag(tag)

    async def on_start(self) -> None:
        self.worker_manager = WorkerManager(self)

    async def on_step(self, iteration: int) -> None:
        if iteration == 0:
            for worker in self.workers:
                worker(AbilityId.STOP_STOP)
            self.worker_manager.distribute_idle_workers(0)
        self.worker_manager.speed_mine()

        if iteration == 100:
            th = self.townhalls.first
            pos_above_th = th.position + Point2((0, 15))
            worker = self.worker_manager.select_worker(pos_above_th, WorkerRole.BUILD)
            worker.move(pos_above_th)