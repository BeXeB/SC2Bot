import math

from sc2.player import Bot, Computer, Human
from sc2.data import Difficulty, Race
from sc2.main import run_multiple_games, GameMatch
from sc2 import maps
from testbot import MyBot, PeacefulBot, ActionSelection
from sc2_mcts import ValueHeuristic, RolloutHeuristic

NUMBER_OF_GAMES: int = 2
GAME_LENGTH: int = 300

test_match = GameMatch(
    maps.get("KingsCoveLE"),
    [Bot(Race.Terran, MyBot(
        mcts_seed=0,
        mcts_rollout_end_time=300,
        mcts_exploration=0.9,
        mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
        mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
        action_selection=ActionSelection.MultiBestActionMin,
        future_action_queue_length=2,
        fixed_search_rollouts=5000)),
     Bot(Race.Zerg, PeacefulBot())],
    realtime=False,
    random_seed=0,
    game_time_limit=GAME_LENGTH,
)

human_match = GameMatch(
    maps.get("KingsCoveLE"),
    [Human(Race.Terran), Bot(Race.Zerg, PeacefulBot())],
    realtime=True,
    random_seed=0,
    game_time_limit=GAME_LENGTH,
)

# EXPERIMENTS
ROLLOUT_END_TIMES: list[int] = [300, 600, 900]
EXPLORATIONS: list[float] = [0.8, 0.9, math.sqrt(2)]
VALUE_HEURISTICS: list[ValueHeuristic] = [ValueHeuristic.UCT, ValueHeuristic.EpsilonGreedy]
ACTION_SELECTIONS: list[ActionSelection] = [ActionSelection.BestAction,
                                            ActionSelection.BestActionFixed,
                                            ActionSelection.MultiBestAction,
                                            ActionSelection.MultiBestActionFixed,
                                            ActionSelection.MultiBestActionMin]
FUTURE_ACTION_QUEUE_LENGTHS: list[int] = [0, 1, 2, 3, 4]

# generate matches
# test each argument separately, with no changes to the others
matches: list[GameMatch] = []
REPEAT_AMOUNT: int = 2
# rollout end tome test, when we look ahead to the end of the game
for ROLLOUT_END_TIME in ROLLOUT_END_TIMES:
    match = GameMatch(
        maps.get("KingsCoveLE"),
        [Bot(Race.Terran, MyBot(
            mcts_seed=0,
            mcts_rollout_end_time=ROLLOUT_END_TIME,
            mcts_exploration=0.9,
            mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
            mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
            action_selection=ActionSelection.MultiBestActionMin,
            future_action_queue_length=2,
            fixed_search_rollouts=5000
        )), Bot(Race.Zerg, PeacefulBot())],
        realtime=False,
        random_seed=0,
        game_time_limit=ROLLOUT_END_TIME,
    )

    for _ in range(REPEAT_AMOUNT):
        matches.append(match)

# rollout end time test, when we dont look ahead to the end of the game
for ROLLOUT_END_TIME in ROLLOUT_END_TIMES:
    if ROLLOUT_END_TIME == ROLLOUT_END_TIMES[-1]:
        continue

    match = GameMatch(
        maps.get("KingsCoveLE"),
        [Bot(Race.Terran, MyBot(
            mcts_seed=0,
            mcts_rollout_end_time=ROLLOUT_END_TIME,
            mcts_exploration=0.9,
            mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
            mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
            action_selection=ActionSelection.MultiBestActionMin,
            future_action_queue_length=2,
            fixed_search_rollouts=5000
        )), Bot(Race.Zerg, PeacefulBot())],
        realtime=False,
        random_seed=0,
        game_time_limit=900,
    )

    for _ in range(REPEAT_AMOUNT):
        matches.append(match)
#
# # exploration test, with 10 min games
# for EXPLORATION in EXPLORATIONS:
#     match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=300,
#             mcts_exploration=EXPLORATION,
#             mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ActionSelection.MultiBestActionMin,
#             future_action_queue_length=2,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=False,
#         random_seed=0,
#         game_time_limit=600,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)
#
# # value heuristic tests, with 10 min games
# for VALUE_HEURISTIC in VALUE_HEURISTICS:
#     match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=300,
#             mcts_exploration=0.9,
#             mcts_value_heuristics=VALUE_HEURISTIC,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ActionSelection.MultiBestActionMin,
#             future_action_queue_length=2,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=False,
#         random_seed=0,
#         game_time_limit=600,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)
#
# # action selection with 10 min games
# for ACTION_SELECTION in ACTION_SELECTIONS:
#     match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=300,
#             mcts_exploration=0.9,
#             mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ACTION_SELECTION,
#             future_action_queue_length=2,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=False,
#         random_seed=0,
#         game_time_limit=600,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)
#
# # future action selection queue length, 10 min games, multibestactionmin
# for FUTURE_ACTION_QUEUE_LENGTH in FUTURE_ACTION_QUEUE_LENGTHS:
#     match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=300,
#             mcts_exploration=0.9,
#             mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ActionSelection.MultiBestActionMin,
#             future_action_queue_length=FUTURE_ACTION_QUEUE_LENGTH,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=False,
#         random_seed=0,
#         game_time_limit=600,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)


# TODO: once we have narrowed down the parameters, try realtime

file = open("result.csv", "w")
file.write("Mcts Seed,"
           "Mcts Rollout Time,"
           "Mcts Exploration,"
           "Mcts Value Heuristic,"
           "Mcts Rollout Heuristic,"
           "Action Selection,"
           "Future Action Queue Length,"
           "Fixed Search Rollouts,"
           "Time,"
           "Minerals,"
           "Total Minerals,"
           "Vespene,"
           "Workers,"
           "Bases,"
           "Gas Buildings," "Score"
           "\n")
file.close()

run_multiple_games(
    matches
)
