from typing import List

from sc2.player import Bot, Computer, Human
from sc2.data import Difficulty, Race, AIBuild
from sc2.main import run_multiple_games, GameMatch
from sc2 import maps
from testbot import MyBot, PeacefulBot, ActionSelection
from sc2_mcts import ValueHeuristic, RolloutHeuristic

NUMBER_OF_GAMES: int = 2
GAME_LENGTH: int = 480
matches: List[GameMatch] = []

match = GameMatch(
        maps.get("KingsCoveLE"),
        [Bot(Race.Terran, MyBot(
            mcts_seed=0,
            mcts_rollout_end_time=GAME_LENGTH,
            mcts_exploration=100,
            mcts_value_heuristics=ValueHeuristic.UCT,
            mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
            action_selection=ActionSelection.MultiBestActionMin,
            future_action_queue_length=2,
            fixed_search_rollouts=5000
        )), Bot(Race.Terran, PeacefulBot())],
        realtime=False,
        disable_fog=True,
        random_seed=0,
        # game_time_limit=GAME_LENGTH,
    )
for _ in range(NUMBER_OF_GAMES):
    matches.append(match)

# EXPERIMENTS
file = open("../result.csv", "w")
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

run_multiple_games(matches)