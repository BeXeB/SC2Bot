import math
import typing
from typing import Optional, Set

from sc2.unit import Unit

from sc2.position import Point2

from Python.CombatScripts.siege_tank_combat import SiegeTankCombat
from Python.CombatScripts.viking_fighter_combat import VikingFighterCombat
from sc2.ids.unit_typeid import UnitTypeId
from sc2.units import Units

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class ArmyManager:
    bot: 'MyBot'
    rally_point: Point2
    unit_exclusion_list: Set[UnitTypeId]
    attacking: bool
    viking_manager: VikingFighterCombat
    tank_manager: SiegeTankCombat

    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot
        self.unit_exclusion_list = self.bot.information_manager.units_to_ignore_for_army
        self.viking_manager = VikingFighterCombat(bot)
        self.tank_manager = SiegeTankCombat(bot)
        self.attacking = False
        self.set_initial_rally_point()
        self.cached_engagements: list[tuple[set[int], list[Unit]]] = []

    def set_initial_rally_point(self):
        closest: Optional[Point2] = None
        min_distance = math.inf
        start_position = self.bot.game_info.player_start_location
        for pos in self.bot.information_manager.expansion_locations.keys():
            distance = start_position.distance_to(pos)
            if min_distance > distance > 5:
                min_distance = distance
                closest = pos
        if closest is not None:
            # Set the rally point to the closest expansion location
            self.set_rally_point(closest.towards(self.bot.game_info.map_center, 5))
        else:
            # Fallback to the start location if no expansion locations are found
            self.set_rally_point(self.bot.start_location)

    def set_rally_point(self, point: Point2) -> None:
        self.rally_point = point

    # TODO: Analyze the map to determine where to attack
    # TODO: Implement squads
    def __units_to_include(self) -> Units:
        return self.bot.units.exclude_type(self.unit_exclusion_list)

    async def manage_army(self) -> None:
        if self.__units_to_include().amount > 20 or self.attacking:
            self.attacking = True
            self.attack_enemy_base()

        if self.__units_to_include().amount < 10 and self.attacking:
            self.attacking = False
            for unit in self.__units_to_include():
                unit.move(self.rally_point)

        #self.defend_structures()
        await self.split_combat_units()

    def defend_structures(self) -> None:
        for building in self.bot.structures:
            if any (self.bot.enemy_units.closer_than(10, building)):
                position = building.position
                combat_units = self.__units_to_include()
                for unit in combat_units:
                    unit.attack(position)
                break

    def attack_enemy(self):
        for unit in self.bot.units:
            if len(self.bot.enemy_units) > 0:
                pos = self.bot.enemy_units[0].position
                self.attack_with_unit(unit, pos)

    def attack_enemy_base(self):
        position = self.bot.enemy_start_locations[0] \
            if self.bot.enemy_structures.empty \
            else self.bot.enemy_structures.closest_to(self.bot.start_location).position

        for unit in self.__units_to_include():
            self.attack_with_unit(unit, position)

    def attack_with_unit(self, unit:Unit , position):
        unit.attack(position)
        match unit.type_id:
            case UnitTypeId.VIKINGFIGHTER:
                self.viking_manager.manage_unit(unit)
            case UnitTypeId.SIEGETANK | UnitTypeId.SIEGETANKSIEGED:
                self.tank_manager.manage_unit(unit)


    async def split_combat_units(self):
        all_enemies = self.bot.enemy_units

        if all_enemies.empty:
            print("No enemies to split against")
            return

        if all_enemies.first.position is None:
            return

        unit_nearest_cc = self.bot.units.closest_to(self.bot.start_location)

        nearest_enemy = all_enemies.closest_to(unit_nearest_cc)

        if nearest_enemy is None:
            return

        enemy_squad = self.bot.enemy_units.closer_than(5, nearest_enemy)

        alive_units = self.bot.units

        # Checks whether enemy_squad is a subset of the cached enemy_squad
        if self.is_squad_cached_and_valid(enemy_squad, alive_units):
            print("already cached that enemy squad")
            enemy_tags = set(unit.tag for unit in enemy_squad)
            for cached_tags, counter_units in self.cached_engagements:
                if enemy_tags == cached_tags:
                    for unit in counter_units:
                        unit.attack(enemy_squad.center)
                    break
            return

        own_buildings = self.bot.structures

        for building in own_buildings:
            if enemy_squad.closer_than(10, building):
                split_units = await self.find_winning_composition(enemy_squad)

                enemy_tags = set(unit.tag for unit in enemy_squad)

                already_cached = any(enemy_tags == cached_tags for cached_tags, _ in self.cached_engagements)

                if not already_cached:

                    # Prune outdated subsets first
                    self.cached_engagements = [
                        (cached_tags, counter_units)
                        for cached_tags, counter_units in self.cached_engagements
                        if not cached_tags < enemy_tags
                    ]

                    # Cache the new superset
                    self.cached_engagements.append((enemy_tags, list(split_units)))

                if split_units.empty:
                    split_units = self.bot.units

                for unit in split_units:
                    print("unit with tag : " + str(unit.tag))
                    unit.attack(enemy_squad.center)
                print("is now attacking enemy squad consisting of: ")
                for enemy in enemy_squad:
                    print(str(enemy.tag))

    def is_squad_cached_and_valid(self, enemy_squad: Units, alive_units: Units) -> bool:
        enemy_tags = set(unit.tag for unit in enemy_squad)
        alive_tags = set(unit.tag for unit in alive_units)

        for cached_tags, counter_units in self.cached_engagements:
            if enemy_tags == cached_tags:
                for unit in counter_units:
                    if unit.tag not in alive_tags:
                        print("Lost counter unit(s) for squad: " + str(enemy_tags))
                        return False
                return True

        return False



    async def find_winning_composition(self, enemy_squad: Units) -> Units:
        own_units = self.bot.units.exclude_type(self.unit_exclusion_list)
        win_prob = self.bot.information_manager.get_combat_win_probability(own_units, enemy_squad)
        own_units = list(own_units)
        final_units = own_units.copy()
        i = 0


        if (win_prob < 0.5):
            print("Army can't win")
            return Units([], self.bot)

        while (i < len(final_units)):
            # Takes the elements before index i and the elements after index i, excluding element i
            test_units = final_units[:i] + final_units[i+1:]

            win_prob = self.bot.information_manager.get_combat_win_probability(Units(test_units, self.bot), enemy_squad)
            if win_prob > 0.5:
                # If our win probability is still above 50%, we keep the new list for further iterations
                final_units = test_units
            else:
                i += 1


        # FOR TESTING PURPOSES PRINT OUTS #
        print("Enemies attacking:")
        for enemy in enemy_squad:
            print(enemy.type_id)
        print("CHOSEN SPLIT UNITS:")
        for unit in final_units:
            print(unit.type_id)

        return Units(final_units, self.bot)
