from sc2.bot_ai import BotAI
# Base = Command center
# Bases should be in proximity of minerals, but no more than 1 for each cluster of minerals
# This is what we should do:
# 1. Search the map for clusters of minerals (Around 10? from the base)
#   Find all minerals and keep them in a set
#   Create a subset from the closest mineral and the minerals in a radius of like 10 of the mineral
# 2. Implicitly "select" the optimal one (Either closest one to current units, or current base?)
#   Find the closest mineral cluster to current base, which is not in the subset of this base's minerals
# 3. Calculate the optimal spot in between the minerals in the cluster to place the base
#   Figure out the optimal placement/math for this
# 4. The parametrized worker should then move to that spot and build a base there.
#   Can possibly simply use the build method to move there and build

# 1.
# Find all minerals and keep them in a set

class BaseBuilder():
    def FindOptimalNewBase(self):
        


    # Only for base rn, but change it to iterate through all the bases
    def CalculateAllCurrentAndRemainingMinerals(self, botai):
        closestMinerals = botai.state.mineral_field.closest_distance_to(botai.start_location)
        currentMinerals = botai.state.mineral_field.closer_than(closestMinerals + 2, botai.start_location)
        allMinerals = botai.state.mineral_field
        remainingMinerals = []
        for mineral in allMinerals:
            if mineral not in currentMinerals:
                remainingMinerals.append(mineral)

        return currentMinerals, remainingMinerals

    def TestCalculations(self, current, remaining):
        print("1-------------------------------------------------------------")
        for mineral in current:
            print(mineral)

        print("2-------------------------------------------------------------")

        for mineral in remaining:
            print(mineral)

