from sc2.position import Point2
from sc2.ids.unit_typeid import UnitTypeId


class BaseBuilder():
    # Initialize the basebuilder to keep track of the occupied clusters, such that we can find the new ones
    def __init__(self, bot):
        self.bot = bot
        self.occupied_clusters = []  # List to keep track of occupied mineral clusters

    async def find_next_base_location(self):
        # Search map for the closest mineral cluster
        all_mineral_fields = self.bot.mineral_field
        existing_bases = self.bot.townhalls

        # Find clusters of minerals with no base
        mineral_clusters = self.group_minerals_into_clusters(all_mineral_fields)

        # Gather unoccupied clusters while ensuring they're not too close to existing bases
        unoccupied_clusters = [
            cluster for cluster in mineral_clusters
            if cluster not in self.occupied_clusters and not any(
                base.distance_to(cluster.center) < 15 for base in existing_bases
            )
        ]

        # If any valid unoccupied clusters exist, choose one for a new base
        if unoccupied_clusters:
            closest_base = self.bot.townhalls.closest_to(self.bot.start_location)

            # Choose the closest cluster to the closest base
            closest_cluster = min(unoccupied_clusters, key=lambda cluster: closest_base.distance_to(cluster.center))

            # Calculate the average position of minerals in the chosen cluster
            cluster_positions = [mineral.position for mineral in closest_cluster.minerals]
            average_position = Point2((sum(pos.x for pos in cluster_positions) / len(cluster_positions),
                                       sum(pos.y for pos in cluster_positions) / len(cluster_positions)))

            # Define the position to place the base
            next_build_position = average_position.offset((1, 0))  # You can adjust this offset

            # Check if the proposed build position is valid
            if await self.is_valid_build_position(next_build_position, existing_bases):
                # Find valid location
                next_build_location = await self.bot.find_placement(UnitTypeId.COMMANDCENTER, next_build_position)

                if next_build_location:
                    self.occupied_clusters.append(closest_cluster)  # Mark this cluster as occupied
                    return next_build_location

        # If there are no valid unoccupied mineral clusters, return None
        return None

    async def is_valid_build_position(self, position, existing_bases):
        # Ensure the position is not too close to existing bases
        if any(base.distance_to(position) < 10 for base in existing_bases):
            return False

        # Check if the position is valid for building
        valid_build = await self.bot.find_placement(UnitTypeId.COMMANDCENTER, position)
        return valid_build is not None

    def group_minerals_into_clusters(self, minerals):
        clusters = []
        for mineral in minerals:
            found_cluster = False
            for cluster in clusters:
                if cluster.center.distance_to(mineral.position) < 10:  # Use the center to check distance
                    cluster.add(mineral)  # Add mineral to the existing cluster
                    found_cluster = True
                    break

            if not found_cluster:
                # Create a new cluster with this mineral
                clusters.append(MineralCluster(mineral))  # Create a new MineralCluster object

        return clusters


class MineralCluster:
    def __init__(self, mineral):
        self.minerals = [mineral]  # Start with the first mineral
        self.center = mineral.position  # Initially, the center is the mineral's position

    def add(self, mineral):
        self.minerals.append(mineral)
        self.update_center()  # Update the center of the cluster

    def update_center(self):
        # Calculate the center position of the cluster based on its minerals
        x = sum(mineral.position.x for mineral in self.minerals) / len(self.minerals)
        y = sum(mineral.position.y for mineral in self.minerals) / len(self.minerals)
        self.center = Point2((x, y))






'''from sc2.position import Point2
from sc2.ids.unit_typeid import UnitTypeId


class BaseBuilder():
    def __init__(self, bot):
        self.bot = bot
        self.occupied_clusters = []  # List to keep track of occupied mineral clusters

    async def find_next_base_location(self):
        # Search map for the closest mineral cluster
        all_mineral_fields = self.bot.mineral_field
        existing_bases = self.bot.townhalls

        # Find clusters of minerals with no base
        mineral_clusters = self.group_minerals_into_clusters(all_mineral_fields)

        # Gather unoccupied clusters while ensuring they're not too close to existing bases
        unoccupied_clusters = [
            cluster for cluster in mineral_clusters
            if cluster not in self.occupied_clusters and not any(
                base.distance_to(cluster.center) < 15 for base in existing_bases
            )
        ]

        # If any valid unoccupied clusters exist, choose one for a new base
        if unoccupied_clusters:
            closest_base = self.bot.townhalls.closest_to(self.bot.start_location)

            # Choose the closest cluster to the closest base
            closest_cluster = min(unoccupied_clusters, key=lambda cluster: closest_base.distance_to(cluster.center))

            # Calculate the average position of minerals in the chosen cluster
            cluster_positions = [mineral.position for mineral in closest_cluster.minerals]
            average_position = Point2((sum(pos.x for pos in cluster_positions) / len(cluster_positions),
                                       sum(pos.y for pos in cluster_positions) / len(cluster_positions)))

            # Define the position to place the base
            next_build_position = average_position.offset((3, 0))  # You can adjust this offset

            # Check if the proposed build position is valid
            if await self.is_valid_build_position(next_build_position, existing_bases):
                # Find valid location
                next_build_location = await self.bot.find_placement(UnitTypeId.COMMANDCENTER, next_build_position)

                if next_build_location:
                    self.occupied_clusters.append(closest_cluster)  # Mark this cluster as occupied
                    return next_build_location

        # If there are no valid unoccupied mineral clusters, return None
        return None

    async def is_valid_build_position(self, position, existing_bases):
        # Ensure the position is not too close to existing bases
        if any(base.distance_to(position) < 10 for base in existing_bases):
            return False

        # Check if the position is valid for building
        valid_build = await self.bot.find_placement(UnitTypeId.COMMANDCENTER, position)
        return valid_build is not None

    def group_minerals_into_clusters(self, minerals):
        clusters = []
        # For example, you can use a simple proximity check
        for mineral in minerals:
            found_cluster = False
            for cluster in clusters:
                if cluster.center.distance_to(mineral.position) < 10:  # Use the center to check distance
                    cluster.add(mineral)  # Add mineral to the existing cluster
                    found_cluster = True
                    break

            if not found_cluster:
                # Create a new cluster with this mineral
                clusters.append(MineralCluster(mineral))  # Create a new MineralCluster object

        return clusters


class MineralCluster:
    def __init__(self, mineral):
        self.minerals = [mineral]  # Start with the first mineral
        self.center = mineral.position  # Initially, the center is the mineral's position

    def add(self, mineral):
        self.minerals.append(mineral)
        self.update_center()  # Update the center of the cluster

    def update_center(self):
        # Calculate the center position of the cluster based on its minerals
        x = sum(mineral.position.x for mineral in self.minerals) / len(self.minerals)
        y = sum(mineral.position.y for mineral in self.minerals) / len(self.minerals)
        self.center = Point2((x, y))'''