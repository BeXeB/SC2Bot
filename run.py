import asyncio

from sc2.player import Bot, Computer
from sc2.data import Difficulty, Race
from sc2.main import run_game, run_multiple_games, GameMatch
from sc2 import maps
from testbot import MyBot, PeacefulBot

# run_game(
#     maps.get("KingsCoveLE"),
#     [Bot(Race.Terran, MyBot()), Bot(Race.Zerg, PeacefulBot())],
#     realtime=False,
# )

NUMBER_OF_GAMES: int = 1

match = GameMatch(
    maps.get("KingsCoveLE"),
    [Bot(Race.Terran, MyBot()), Bot(Race.Zerg, PeacefulBot())],
    realtime=False,
    random_seed=0,
)

run_multiple_games(
    [match for _ in range(NUMBER_OF_GAMES)],
)