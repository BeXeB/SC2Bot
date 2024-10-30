# Base = Command center
# Bases should be in proximity of minerals, but no more than 1 for each cluster of minerals
# This is what we should do:
# 1. Search the map for clusters of minerals
# 2. Implicitly "select" the optimal one (Either closest one to current units, or current base?)
# 3. Calculate the optimal spot in between the minerals in the cluster to place the base
# 4. The parametrized worker should then move to that spot and build a base there.

# 1.




# Different ideas:
# Either we cluster the mineral fields together based
# on whether 2 or more minerals are close to each other
# or we sort it based on which minerals are not currently
# being mined (or are in the vicinity of a base) and
# are closest to us. (sorted_by_distance_to())