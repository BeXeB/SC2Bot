import math
import typing

from sc2.position import Point2
from sc2.ids.unit_typeid import UnitTypeId
from typing import List, Optional

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

from sc2.unit import Unit
from sc2.units import Units


class BaseBuilder():
    # Initialize the basebuilder to keep track of the occupied clusters, such that we can find the new ones
    def __init__(self, bot: 'MyBot') -> None:
        self.bot = bot
        self.occupied_clusters: List[MineralCluster] = []

    async def find_next_base_location(self) -> Optional[Point2]:
        closest = None
        min_distance = math.inf
        for expansion_location in self.bot.information_manager.expansion_locations:
            if self.bot.information_manager.expansion_locations[expansion_location]:
                # already taken
                continue

            start_position = self.bot.game_info.player_start_location
            distance = await self.bot.client.query_pathing(start_position, expansion_location)
            if distance is None:
                continue

            if distance < min_distance:
                min_distance = distance
                closest = expansion_location

        return closest

    # Not going to remove this, as this is a good idea, but it has issues with the current implementation
    # async def find_next_base_location(self) -> Optional[Point2]:
    #     # Search map for the closest mineral cluster
    #     all_mineral_fields = self.bot.mineral_field
    #     existing_bases = self.bot.townhalls
    #
    #     # Find clusters of minerals with no base
    #     mineral_clusters = self.__group_minerals_into_clusters(all_mineral_fields)
    #
    #     # Gather unoccupied clusters while ensuring they're not too close to existing bases (Arbitrary number of 15)
    #     unoccupied_clusters = [
    #         cluster for cluster in mineral_clusters
    #         if cluster not in self.occupied_clusters and not any(
    #             base.distance_to(cluster.center) < 15 for base in existing_bases
    #         )
    #     ]
    #
    #     # If any valid unoccupied clusters exist, choose one for a new base
    #     if unoccupied_clusters:
    #         closest_base = self.bot.townhalls.closest_to(self.bot.start_location)
    #
    #         # Choose the closest cluster to the closest base
    #         closest_cluster = min(unoccupied_clusters, key=lambda cluster: closest_base.distance_to(cluster.center))
    #
    #         # Average position
    #         average_position = closest_cluster.center
    #
    #
    #         # Define the position to place the base with an x-value offset
    #         next_build_position = average_position.offset((1, 0))
    #
    #         # Check if the proposed build position is valid
    #         if await self.__is_valid_build_position(next_build_position, existing_bases):
    #             # Find valid location
    #             next_build_location = await self.bot.find_placement(UnitTypeId.COMMANDCENTER, next_build_position)
    #
    #             if next_build_location:
    #                 # Mark this cluster as occupied
    #                 self.occupied_clusters.append(closest_cluster)
    #                 return next_build_location
    #
    #     # If there are no valid unoccupied mineral clusters, return None
    #     return None

    async def __is_valid_build_position(self, position: Point2, existing_bases: Units) -> bool:
        # Ensure the position is not too close to existing bases
        if any(base.distance_to(position) < 10 for base in existing_bases):
            return False

        # Check if the position is valid for building
        valid_build = await self.bot.find_placement(UnitTypeId.COMMANDCENTER, position)
        return valid_build is not None

    def __group_minerals_into_clusters(self, minerals: Units) -> List['MineralCluster']:
        clusters = []
        for mineral in minerals:
            found_cluster = False
            for cluster in clusters:
                # Use the center to check distance
                if cluster.center.distance_to(mineral.position) < 10:
                    # Add mineral to the existing cluster
                    cluster.add(mineral)
                    found_cluster = True
                    break

            if not found_cluster:
                # Create a new cluster with this mineral
                clusters.append(MineralCluster(mineral))

        return clusters


class MineralCluster:
    def __init__(self, mineral: Unit) -> None:
        # Start with the first mineral
        self.minerals = [mineral]
        # Initially, the center is the mineral's position
        self.center = mineral.position

    def add(self, mineral: Unit) -> None:
        self.minerals.append(mineral)
        # Update the center of the cluster
        self.update_center()

    def update_center(self) -> None:
        # Calculate the center position of the cluster based on its minerals
        x = sum(mineral.position.x for mineral in self.minerals) / len(self.minerals)
        y = sum(mineral.position.y for mineral in self.minerals) / len(self.minerals)
        self.center = Point2((x, y))
