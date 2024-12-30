import math
import sys

from sc2.player import Bot, Computer, Human
from sc2.data import Difficulty, Race
from sc2.main import run_multiple_games, GameMatch
from sc2 import maps

from better_human import BetterHuman
from testbot import MyBot, PeacefulBot, ActionSelection
from sc2_mcts import ValueHeuristic, RolloutHeuristic

GAME_LENGTH = 60*8
ROLLOUT_END_TIMES: list[int] = [300, 60*8]
EXPLORATIONS: list[float] = [0.5, 0.9, math.sqrt(2)]
VALUE_HEURISTICS: list[ValueHeuristic] = [ValueHeuristic.UCT, ValueHeuristic.EpsilonGreedy]
ACTION_SELECTIONS: list[ActionSelection] = [ActionSelection.BestAction,
                                            ActionSelection.MultiBestAction,
                                            ActionSelection.MultiBestActionMin]
FUTURE_ACTION_QUEUE_LENGTHS: list[int] = [2, 3, 4]

# generate matches
# test each argument separately, with no changes to the others
matches: list[GameMatch] = []
REPEAT_AMOUNT: int = 5

# rollout end time test, when we dont look ahead to the end of the game
# for ROLLOUT_END_TIME in ROLLOUT_END_TIMES:
#
#     match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=ROLLOUT_END_TIME,
#             mcts_exploration=0.9,
#             mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ActionSelection.MultiBestActionMin,
#             future_action_queue_length=2,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=False,
#         random_seed=0,
#         game_time_limit=GAME_LENGTH,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)
# #
# # # exploration test, with 10 min games
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
#         game_time_limit=GAME_LENGTH,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)
# #
# # # value heuristic tests, with 10 min games
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
#         game_time_limit=GAME_LENGTH,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)
#
# # # action selection with 10 min games
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
#         game_time_limit=GAME_LENGTH,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)
# #
# # # future action selection queue length, 10 min games, multibestactionmin
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
#         game_time_limit=GAME_LENGTH,
#     )
#
#     for _ in range(REPEAT_AMOUNT):
#         matches.append(match)


# TODO: once we have narrowed down the parameters, try realtime
# Epsilon Greedy, 0.9, BestAction, 300 rollout end time
# Epsilon Greedy, 0.9, MultiBestAction, queue 3, 300 rollout end time
# UCT, 2, BestAction, 300 rollout end time

# match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=300,
#             mcts_exploration=0.9,
#             mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ActionSelection.BestAction,
#             future_action_queue_length=0,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=True,
#         random_seed=0,
#         game_time_limit=GAME_LENGTH,
#     )
#
# for _ in range(REPEAT_AMOUNT):
#     matches.append(match)
#
# match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=300,
#             mcts_exploration=0.9,
#             mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ActionSelection.MultiBestAction,
#             future_action_queue_length=3,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=True,
#         random_seed=0,
#         game_time_limit=GAME_LENGTH,
#     )
#
# for _ in range(REPEAT_AMOUNT):
#     matches.append(match)
#
# match = GameMatch(
#         maps.get("KingsCoveLE"),
#         [Bot(Race.Terran, MyBot(
#             mcts_seed=0,
#             mcts_rollout_end_time=300,
#             mcts_exploration=2,
#             mcts_value_heuristics=ValueHeuristic.UCT,
#             mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#             action_selection=ActionSelection.BestAction,
#             future_action_queue_length=0,
#             fixed_search_rollouts=5000
#         )), Bot(Race.Zerg, PeacefulBot())],
#         realtime=True,
#         random_seed=0,
#         game_time_limit=GAME_LENGTH,
#     )
#
# for _ in range(REPEAT_AMOUNT):
#     matches.append(match)

matches.append(GameMatch(
        maps.get("KingsCoveLE"),
        [Bot(Race.Terran, BetterHuman()), Bot(Race.Zerg, PeacefulBot())],
        realtime=True,
        random_seed=0,
        sc2_config=[{"fullscreen": True}, {}],
        game_time_limit=GAME_LENGTH,
    ))

game_index_to_run = int(sys.argv[1])

if game_index_to_run < len(matches):
    run_multiple_games(
        [matches[game_index_to_run]],
    )