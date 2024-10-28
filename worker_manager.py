from enum import Enum

from sc2.ids.unit_typeid import UnitTypeId

class WorkerManager:
    def __init__(self, bot):
        self.bot = bot
        workers = bot.get_units(UnitTypeId.SCV)
        self.workers = workers.for_each(lambda w: (w, WorkerState.Idle))

    def distribute_workers(self, ratio):
        for worker in self.workers:
            if worker[1] == WorkerState.Idle:
                # Check if we need more workers on minerals or gas
                # Change this so it makes sense, we should be able to distribute workers based on a ratio of minerals to gas
                # Maybe find the number of workers on minerals and gas and compare that to the ratio
                # Also check if there is a base that needs workers
                # If there is a base that needs workers, send a worker to that base
                # Check if gas is full, if it is, send a worker to minerals
                if 1 > ratio:
                    worker[1] = WorkerState.GatheringMinerals
                    self.bot.do(worker[0].gather(self.bot.mineral_field.closest_to(worker[0])))
                else:
                    worker[1] = WorkerState.GatheringGas
                    self.bot.do(worker[0].gather(self.bot.vespene_geyser.closest_to(worker[0])))

class WorkerState(Enum):
    Idle = 0
    GatheringMinerals = 1
    ReturningMinerals = 2
    GatheringGas = 3
    ReturningGas = 4
    Building = 5
    Repairing = 6
    Attacking = 7
    Scouting = 8
    Fleeing = 9