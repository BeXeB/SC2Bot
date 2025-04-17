import sys
from typing import List

from Python.battle_bot import BattleBot
from sc2.player import Bot, Computer
from sc2.data import Difficulty, Race
from sc2.main import run_multiple_games, run_game, GameMatch
from sc2 import maps
from Python.testbot import MyBot, ActionSelection, PeacefulBot
from sc2_mcts import ValueHeuristic, RolloutHeuristic
from __init__ import run_ladder_game  # Import ladder game function

NUMBER_OF_GAMES = 1
GAME_LENGTH = 480

# bot = Bot(Race.Terran, MyBot(
#     mcts_seed=0,
#     mcts_rollout_end_time=GAME_LENGTH,
#     mcts_exploration=0.8,
#     mcts_value_heuristics=ValueHeuristic.EpsilonGreedy,
#     mcts_rollout_heuristics=RolloutHeuristic.weighted_choice,
#     action_selection=ActionSelection.MultiBestActionMin,
#     future_action_queue_length=2,
#     minimum_search_rollouts=5000
# ))

bot = Bot(Race.Terran, BattleBot())

if __name__ == "__main__":

    if "--LadderServer" in sys.argv:
        print("Starting ladder game...")
        result, opponentid = run_ladder_game(bot)
        print(result, " against opponent ", opponentid)
    else:
        print("Starting local game...")
        opponent = Computer(Race.Terran, Difficulty.MediumHard)
        #opponent = Bot(Race.Terran, PeacefulBot())
        match = GameMatch(
            maps.get("KingsCoveLE"),
            [opponent,bot],
            realtime=False,
            disable_fog=False,
            random_seed=0
        )
        run_multiple_games([match])
