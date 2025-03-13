import typing

from sc2.pixel_map import PixelMap

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot


class MapAnalyzer:
    def __init__(self, bot: 'MyBot'):
        self.bot = bot

    def print_map(self):
        grid: PixelMap = self.bot.game_info.placement_grid
        print(grid.width)
        print(grid.height)
        grid.save_image("asd.png")
