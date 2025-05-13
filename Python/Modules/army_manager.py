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
        self.cached_engagements: list[CachedEngagements] = []#: list[tuple[set[int], list[Unit]]] = []

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
        # Check whether we have units or not
        if self.bot.units.empty:
            return

        all_enemies = self.bot.enemy_units

        # Check if any enemies exist
        if all_enemies.empty:
            print("No enemies to split against")
            return

        # I'm not completely sure why, but sometimes checking whether enemies is empty or not is not enough
        # I've put this check in to ensure that the first enemy has a position, as this ensures we can find an
        # existing enemy with valid values
        if all_enemies.first.position is None:
            return

        unit_nearest_cc = self.bot.units.closest_to(self.bot.start_location)

        nearest_enemy = all_enemies.closest_to(unit_nearest_cc)

        # Double check. Might be removable, but during testing it complained
        if nearest_enemy is None:
            return

        enemy_squad = self.bot.enemy_units.closer_than(10, nearest_enemy)
        enemy_tags = set(unit.tag for unit in enemy_squad)

        alive_units = self.bot.units


        # Checks whether enemy_squad is a subset of the cached enemy_squad
        if self.is_squad_cached_and_valid(enemy_squad, alive_units):
            print("already cached that enemy squad")
            engagement = self.get_engagement_by_tags(enemy_tags)
            for unit in engagement.counter_units:
                unit.attack(enemy_squad.center)
            return

        own_buildings = self.bot.structures

        for building in own_buildings:
            if enemy_squad.closer_than(10, building):
                # Method 1
                # If the new enemy squad is a superset of the cached one, augment the counter_forces instead of recalculating
                # win_prob = self.bot.information_manager.get_combat_win_probability(own_units, enemy_squad)
                split_units = await self.augment_current_counter_forces(enemy_squad)

                # Method 2
                # If the new enemy squad is completely new, recalculate
                if split_units is None:
                    print("AUGMENTED WAS SKIPPED")
                    split_units = await self.recalculate_split(enemy_squad)

                if split_units.empty:
                    print("RECALCULATION WAS EMPTY")
                    split_units = self.bot.units

                for unit in split_units:
                    print("unit with tag: " + str(unit.tag) + ", " + str(unit.type_id))
                    unit.attack(enemy_squad.center)
                print("is now attacking enemy squad consisting of: ")
                for enemy in enemy_squad:
                    print(str(enemy.tag) + ", " + str(enemy.type_id))


    def is_squad_cached_and_valid(self, enemy_squad: Units, alive_units: Units):
        enemy_tags = set(unit.tag for unit in enemy_squad)
        alive_tags = set(unit.tag for unit in alive_units)

        for engagement in self.cached_engagements:
            if set(engagement.cached_tags) != enemy_tags:
                continue
            if all(unit.tag in alive_tags for unit in engagement.counter_units):
                return True
            print("Lost counter unit(s) for squad: " + str(enemy_tags))
            return False

    def get_engagement_by_tags(self, enemy_tags: set[int]):
        for engagement in self.cached_engagements:
            if set(engagement.cached_tags) == enemy_tags:
                return engagement
        return None

    # If the new enemy squad is a superset of the cached one, augment the counter_forces instead of recalculating
    # win_prob = self.bot.information_manager.get_combat_win_probability(own_units, enemy_squad)
    async def augment_current_counter_forces(self, enemy_squad: Units):
        enemy_tags = set(unit.tag for unit in enemy_squad)
        subset_engagement = self.get_subset_engagement(enemy_tags)
        # If the new enemy squad is not a subset of a current one, we keep the current counter forces
        if not subset_engagement:
            return None

        # Exclude already used units, such that we don't use counter forces from other splits
        used_tags = {unit.tag for unit in subset_engagement.counter_units}
        available_units = [unit for unit in self.__units_to_include() if unit.tag not in used_tags]

        augmented_units = list(subset_engagement.counter_units)

        # Recalculate the winprob with new augmented units
        for unit in available_units:
            augmented_units.append(unit)
            win_prob = self.bot.information_manager.get_combat_win_probability(Units(augmented_units, self.bot), enemy_squad)
            if win_prob > 0.7:
                break

        # If we still cannot completely beat the enemy squad, fall back to full recalculation
        final_win_prob = self.bot.information_manager.get_combat_win_probability(Units(augmented_units, self.bot), enemy_squad)
        if final_win_prob < 0.7:
            return None

        # Remove old subset engagement
        self.cached_engagements = [
            engagement for engagement in self.cached_engagements
            if set(engagement.cached_tags) != set(subset_engagement.cached_tags)
        ]

        # Cache the augmented set
        self.cached_engagements.append(CachedEngagements(list(enemy_tags), augmented_units))

        return Units(augmented_units, self.bot)



    async def recalculate_split(self, enemy_squad: Units):
        enemy_tags = set(unit.tag for unit in enemy_squad)

        split_units = await self.find_winning_composition(enemy_squad)

        # Prune outdated subsets
        self.cached_engagements = [
            engagement for engagement in self.cached_engagements
            if not set(engagement.cached_tags) < enemy_tags
        ]

        self.cached_engagements.append(CachedEngagements(list(enemy_tags), list(split_units)))

        return split_units


    def get_subset_engagement(self, enemy_tags: set[int]):
        for engagement in self.cached_engagements:
            if set(engagement.cached_tags) < enemy_tags:
                return engagement
        return None

    async def find_winning_composition(self, enemy_squad: Units) -> Units:
        own_units = self.__units_to_include()
        win_prob = self.bot.information_manager.get_combat_win_probability(own_units, enemy_squad)
        own_units = list(own_units)
        final_units = own_units.copy()
        i = 0


        if (win_prob < 0.7):
            print("Army can't win")
            # Currently, if the army can't win, we send EVERYTHING, SCV's included, at the enemy.
            # For future purposes, we should probably still exclude this (Can be easily done), such that the
            # SCV's could focus on retreating and gathering minerals somewhere. But for now, it is an alright way
            # to handle stuff like worker rush
            return Units([], self.bot)

        while (i < len(final_units)):
            # Takes the elements before index i and the elements after index i, excluding element i
            test_units = final_units[:i] + final_units[i+1:]

            win_prob = self.bot.information_manager.get_combat_win_probability(Units(test_units, self.bot), enemy_squad)
            if win_prob > 0.7:
                # If our win probability is still above 70%, we keep the new list for further iterations
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


class CachedEngagements:
    cached_tags: list[Unit.tag]
    counter_units: list[Unit]

    def __init__(self, cached_tags: list[Unit.tag], counter_units: list[Unit]):
        self.cached_tags = cached_tags
        self.counter_units = counter_units


# self.cached_engagements: list[tuple[set[int], list[Unit]]] = []