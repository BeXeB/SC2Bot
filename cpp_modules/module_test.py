import sys
import os

sys.path.append(os.path.abspath('vsstudio/Release'))

# noinspection PyUnresolvedReferences
import sc2_mcts

state = sc2_mcts.sc2_state()
state.id = 5

print(state.to_string())
print("minerals: " + str(state.get_minerals()))
print("vespene: " + str(state.get_vespene()))
print("incoming population: " + str(state.get_incoming_population()))
print("Population limit: " + str(state.get_population_limit()))
print("Population: " + str(state.get_population()))
print("get_occupied_population: " + str(state.get_occupied_population()))
print("mineral per timestep: " + str(state.mineral_gained_per_time_step()))
print("vespene per timestep: " + str(state.vespene_gained_per_time_step()))
print("mineral workers: " + str(state.get_mineral_workers()))
print("vespene workers: " + str(state.get_vespene_workers()))
