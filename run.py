import math

from sc2.player import Bot, Computer
from sc2.data import Difficulty, Race
from sc2.main import run_multiple_games, GameMatch
from sc2 import maps
from testbot import MyBot, PeacefulBot, ActionSelection
from sc2_mcts import ValueHeuristic, RolloutHeuristic

NUMBER_OF_GAMES: int = 2
GAME_LENGTH: int = 300

match = GameMatch(
    maps.get("KingsCoveLE"),
    [Bot(Race.Terran, MyBot(
        mcts_seed = 0,
        mcts_rollout_end_time = 300,
        mcts_exploration = 0.9,
        mcts_value_heuristics = ValueHeuristic.EpsilonGreedy,
        mcts_rollout_heuristics = RolloutHeuristic.weighted_choice,
        action_selection = ActionSelection.MultiBestActionMin,
        future_action_queue_length = 2,
        fixed_search_rollouts = 5000)),
     Bot(Race.Zerg, PeacefulBot())],
    realtime = False,
    random_seed = 0,
    game_time_limit = GAME_LENGTH,
)

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
           # "Actions,"
           # "Iterations"
           "\n")

file.close()

run_multiple_games(
    [match for _ in range(NUMBER_OF_GAMES)],
)