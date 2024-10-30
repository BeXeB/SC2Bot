from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId

# Base = Command center
# Bases should be in proximity of minerals, but no more than 1 for each cluster of minerals
# This is what we should do:
# 1. Search the map for clusters of minerals
#   Find all minerals and keep them in a set
#   Create a subset from the closest mineral and the minerals in a radius of like 10 of the mineral
# 2. Implicitly "select" the optimal one (Either closest one to current units, or current base?)
#   Find the closest mineral cluster to current base, which is not in the subset of this base's minerals
# 3. Calculate the optimal spot in between the minerals in the cluster to place the base
#   Figure out the optimal placement/math for this
# 4. The parametrized worker should then move to that spot and build a base there.
#   Can possibly simply use the build method to move there and build

class BaseBuilder():
    def FindOptimalNewBase(self, botai):
        print("deez")
        # Brug remaining fra CalculateAllCurrent... til at finde tætteste cluster til næste base
        # Udregn det optimale center i midten af clusteret
        # Byg COMMANDCENTER


        # Måske hav lokal variabel til oldCurrentMinerals, og kald disse explicit i testbot, samt herover og -under.


    # Only for base rn, but change it to iterate through all the bases (IMPORTANT TO KEEP UP WITH ITERATIONS)
    # Should also be changed to select the minerals in a radius, rather than from base. This method is highly specific
    # (Second part could be done by taking the closest, and doing the "closer than" on it)
    def CalculateAllCurrentAndRemainingMinerals(self, botai, oldCurrentMinerals):
        if oldCurrentMinerals == []:
            closestMinerals = botai.state.mineral_field.closest_distance_to(botai.start_location)
            currentMinerals = botai.state.mineral_field.closer_than(closestMinerals + 2, botai.start_location)
        else:
            closestMinerals = botai.state.mineral_field.closest_distance_to(oldCurrentMinerals[0])
            currentMinerals = botai.state.mineral_field.closer_than(closestMinerals + 2, oldCurrentMinerals[0])

        allMinerals = botai.state.mineral_field
        remainingMinerals = oldCurrentMinerals + []
        for mineral in allMinerals:
            if mineral not in currentMinerals or oldCurrentMinerals:
                remainingMinerals.append(mineral)


        return currentMinerals, remainingMinerals

    def TestCalculations(self, current, remaining):
        print("1-------------------------------------------------------------")
        for mineral in current:
            print(mineral)

        print("2-------------------------------------------------------------")

        for mineral in remaining:
            print(mineral)

# Todo:
#