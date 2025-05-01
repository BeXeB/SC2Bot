import math
import typing
import itertools
from sc2.unit import Unit, UnitTypeId
from sc2.units import Units
from sc2.position import Point2
from typing import List, Set
if typing.TYPE_CHECKING:
    from Python.testbot import MyBot
from Python.Modules.information_manager import WorkerRole


class ScoutManager:
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot
        self.cluster_points: List[Point2] = self.bot.expansion_locations_list
        self.cluster_iter = itertools.cycle(self.cluster_points)
        self.target: Point2 = self.cluster_points[0]
        self.scout: Unit | None = None
        self.KITING_RANGE = 5
        self.orbit_points: List[Point2] = []
        self.orbit_iter = itertools.cycle(self.orbit_points)
        self.orbit_target: Point2 = self.target
        self.orbiting = False
        self.dangerous_structures: Set[UnitTypeId] = bot.information_manager.production_powers.keys() - {UnitTypeId.HATCHERY, UnitTypeId.LAIR, UnitTypeId.HIVE}

    def manage_scouts(self) -> None:
        scout_list = self.bot.information_manager.get_workers(WorkerRole.SCOUT)
        if not scout_list:
            self.scout = self.bot.worker_manager.select_worker(self.target, WorkerRole.SCOUT)
            if self.scout is not None:
                self.bot.worker_manager.assign_worker(self.scout.tag, WorkerRole.SCOUT, None)
                self.__sort_expansion_distances()
                self.__update_target()
        if self.scout is not None:
            self.scout = self.bot.workers.by_tag(self.scout.tag)
            self.__kite_scout(self.scout)

    def __kite_scout(self, scout: Unit) -> None:
        enemy_units = self.bot.enemy_units + self.bot.enemy_structures

        # If not already orbiting and the scout is not at the target, move to the target
        if not self.orbiting and scout.distance_to(self.target) > 5:
            scout.move(self.target)
        # Otherwise, if the scout is not in danger, check if there are enemies in range
        # If there are, orbit the target
        # If there are not, update the target
        elif not self.__in_danger(scout, enemy_units):
            if self.__enemies_in_range(enemy_units, scout):
                self.__handle_orbiting(scout)
            else:
                self.__update_target()
        # Otherwise, if the scout is in danger, go to the next target
        elif scout.distance_to(self.target) < 10:
            self.__update_target()

    def __handle_orbiting(self, scout: Unit) -> None:
        self.orbiting = True
        if scout.distance_to(self.orbit_target) < 2:
            self.orbit_target = next(self.orbit_iter)
        scout.move(self.orbit_target)

    def __in_danger(self, scout: Unit, enemy_units: Units) -> bool:
        # if there are enemy units, check if they are dangerous
        if not enemy_units:
            return False
        if self.__dangerous_enemies_in_range(enemy_units, scout):
            # check if the scout is in danger
            return True
        return False

    def __update_target(self) -> None:
        initial_position = self.target
        self.orbiting = False
        while True:
            self.target = next(self.cluster_iter)
            self.orbit_points = self.__calculate_orbit_points()
            self.orbit_iter = itertools.cycle(self.orbit_points)
            self.orbit_target = next(self.orbit_iter)
            if not self.bot.is_visible(self.target):
                break
            if self.target == initial_position:
                break

    def __calculate_orbit_points(self) -> List[Point2]:
        points: List[Point2] = []
        for i in range(0, 360, 30):
            angle = math.radians(i)
            x = self.target.x + 5 * math.cos(angle)
            y = self.target.y + 5 * math.sin(angle)
            points.append(Point2((x, y)))
        return points

    def __enemies_in_range(self, enemies: Units, scout: Unit) -> bool:
        for enemy in enemies:
            if enemy.distance_to(scout) < enemy.ground_range + self.KITING_RANGE:
                return True
        return False

    def __dangerous_enemies_in_range(self, enemies: Units, scout: Unit) -> bool:
        for enemy in enemies:
            if enemy.distance_to(scout) < enemy.ground_range + self.KITING_RANGE:
                if enemy.is_structure and enemy.type_id in self.dangerous_structures and enemy.is_active:
                    return True
                if not enemy.is_structure and enemy.type_id not in self.bot.information_manager.units_to_ignore_for_army:
                    return True
        return False

    def __sort_expansion_distances(self) -> None:
        self.cluster_points.sort(reverse=True, key=self.__sorting_helper)

    def __sorting_helper(self, element) -> float:
        if self.scout is None:
            self.bot.client.chat_send("Scout is None", True)
            return 0
        return self.scout.distance_to(element)

# TODO
# 4. Variable amount of scouts + Variable type (Reaper)