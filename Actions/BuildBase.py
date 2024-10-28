# Base = Command center
# Bases should be in proximity of minerals, but no more than 1 for each cluster of minerals
# This is what we should do:
# 1. Search the map for clusters of minerals
# 2. Implicitly "select" the optimal one (Either closest one to current units, or current base?)
# 3. Calculate the optimal spot in between the minerals in the cluster to place the base
# 4. The parametrized worker should then move to that spot and build a base there.

# 1.
