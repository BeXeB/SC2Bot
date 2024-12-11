from sc2_mcts import *

minerals = 20
vespene = 2
population = 20
incoming_population = 0
population_limit = 25
bases = [
    Base(1, 4, 2, 1),
    Base(1, 4, 2, 1)
]
constructions = [
    Construction(1, Action.build_worker),
    Construction(3, Action.build_worker)
]

bases.append(Base(id=1, mineral_fields=4, vespene_geysers=2, vespene_collectors=1))

occupied_worker_timers = [1, 2, 3]

state : State = state_builder(minerals=minerals,
                              vespene=vespene,
                              population=population,
                              incoming_population=incoming_population,
                              population_limit=population_limit,
                              bases=bases,
                              constructions=constructions,
                              occupied_worker_timers=occupied_worker_timers)

print(state.to_string())

state.id = 5
mcts : Mcts = Mcts(state, 200, 1000, 2, ValueHeuristic.UCT, RolloutHeuristic.weighted_choice)

mcts.update_root_state(state)

for i in range(10) :
    mcts.search_rollout(1000)
    action = mcts.get_best_action()
    mcts.perform_action(action)
    print(action)

print(mcts.get_root_state().to_string())



