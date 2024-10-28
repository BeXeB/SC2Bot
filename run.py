from sc2.player import Bot, Computer
from sc2.data import Difficulty, Race
from sc2.main import run_game
from sc2 import maps
from testbot import MyBot


run_game(
    maps.get("CollectMineralsAndGas"),
    [Bot(Race.Zerg, MyBot()), Computer(Race.Zerg, Difficulty.Hard)],
    realtime=False,
)