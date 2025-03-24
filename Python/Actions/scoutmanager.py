import typing
import itertools
from sc2.unit import Unit, UnitTypeId
from sc2.units import Units
from sc2.position import Point2
if typing.TYPE_CHECKING:
    from Python.testbot import MyBot
from Python.Modules.information_manager import WorkerRole


class ScoutManager:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot
        self.cluster_points: [Point2] = self.bot.expansion_locations_list
        self.cluster_iter = itertools.cycle(self.cluster_points)
        self.target: Point2 = self.cluster_points[0]
        self.scout: Unit | None = None
        self.KITING_RANGE = 5

    def kite_scout(self, scout: Unit) -> None:
        enemy_units = self.bot.enemy_units
        if self.enemies_in_range(enemy_units, self.scout):
            self.update_target()
        if scout.distance_to(self.target) < 5:
            self.update_target()
        scout.move(self.target)

    def update_target(self):
        initial_position = self.target
        while True:
            self.target = next(self.cluster_iter)
            if not self.bot.is_visible(self.target):
                break
            if self.target == initial_position:
                break


    def enemies_in_range(self, enemies: Units, scout: Unit) -> bool:
        for enemy in enemies:
            if enemy.distance_to(scout) < enemy.ground_range + self.KITING_RANGE:
                return True
        return False


    def manage_scouts(self):
        scout_list = self.bot.information_manager.get_workers(WorkerRole.SCOUT)
        if not scout_list:
            self.scout = self.bot.worker_manager.select_worker(self.target, WorkerRole.SCOUT)
            if self.scout is not None:
                self.bot.worker_manager.assign_worker(self.scout.tag, WorkerRole.SCOUT, None)
            self.sort_expansion_distances()
            self.target = next(self.cluster_iter)
        if self.scout is not None:
            self.scout = self.bot.workers.by_tag(self.scout.tag)
            self.kite_scout(self.scout)

    def sort_expansion_distances(self):
        self.cluster_points.sort(reverse=True, key=self.sorting_helper)

    def sorting_helper(self, element):
        return self.scout.distance_to(element)

# TODO
# 1. Rækkefølgen vi undersøger locations
# 4. Variabel antal af scouts + Variabel type (Reaper)

# Done
# 2. Fix når vi kommer for tæt på fjender -> Tilføj bonus distance til når vi møder fjender
# 3. Scout ikke baser vi selv har (Fjern dem fra location listen)
