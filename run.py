from sc2.player import Bot, Computer
from sc2.data import Difficulty, Race
from sc2.main import run_game
from sc2 import maps
from testbot import MyBot, PeacefulBot


run_game(
    maps.get("Simple64"),
    [Bot(Race.Terran, MyBot()), Bot(Race.Terran, PeacefulBot())],
    realtime=False,
)