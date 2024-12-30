import math
import subprocess

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

for i in range(1):
    args = [str(i)]
    subprocess.run(["venv/Scripts/python", "run_games.py", *args])