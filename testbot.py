from sc2.bot_ai import BotAI
from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.unit import Unit
from sc2.units import Units

from Actions.build_supply import SupplyBuilder
from worker_manager import WorkerManager, WorkerRole, TownhallData, GasBuildingData



class MyBot(BotAI):
    worker_manager = None
    supply_builder = None

    def get_units(self, unit_type: UnitTypeId) -> Units:
        return self.units.filter(lambda u: u.type_id == unit_type)

    def get_unit_by_tag(self, tag: int) -> Unit:
        return self.units.find_by_tag(tag)

    async def on_start(self) -> None:
        self.worker_manager = WorkerManager(self)
        self.supply_builder = SupplyBuilder(self)

    async def on_step(self, iteration: int) -> None:
        if iteration == 0:
            for worker in self.workers:
                worker(AbilityId.STOP_STOP)
            for townhall in self.townhalls:
                townhall(AbilityId.RALLY_WORKERS, self.start_location)
        self.worker_manager.distribute_workers()
        self.worker_manager.speed_mine()


        if self.can_afford(UnitTypeId.SCV) and self.supply_left > 0:
            self.train(UnitTypeId.SCV)

        if self.can_afford(UnitTypeId.SUPPLYDEPOT) and self.supply_cap < 200:
            await self.supply_builder.build_supply()

        if self.can_afford(UnitTypeId.COMMANDCENTER) and self.townhalls.amount < 2:
            build_location = None
            for el in self.expansion_locations_list:
                if self.start_location.distance_to(el) < 5:
                    continue
                if build_location is None:
                    build_location = el
                elif self.start_location.distance_to(el) < self.start_location.distance_to(build_location):
                    build_location = el
            worker = self.worker_manager.select_worker(build_location, WorkerRole.BUILD)
            if worker:
                worker.build(UnitTypeId.COMMANDCENTER, build_location)

        if self.can_afford(UnitTypeId.REFINERY) and self.gas_buildings.amount < 2:
            for townhall in self.townhalls:
                vgs = self.vespene_geyser.closer_than(10, townhall)
                for vg in vgs:
                    if self.gas_buildings.filter(lambda u: u.position.distance_to(vg) < 1):
                        continue
                    worker = self.worker_manager.select_worker(vg.position, WorkerRole.BUILD)
                    if worker:
                        await self.build(UnitTypeId.REFINERY, vg, build_worker=worker)
                        worker(AbilityId.STOP_STOP, queue=True)

    async def on_building_construction_complete(self, unit: Unit) -> None:
        if unit.type_id == UnitTypeId.COMMANDCENTER:
            self.worker_manager.th_data.update({unit.tag: TownhallData()})
        if unit.type_id == UnitTypeId.REFINERY:
            self.worker_manager.gas_data.update({unit.tag: GasBuildingData()})
        if unit.type_id == UnitTypeId.SUPPLYDEPOT:
            unit(AbilityId.MORPH_SUPPLYDEPOT_LOWER)

class PeacefulBot(BotAI):
    async def on_step(self, iteration: int) -> None:
        pass