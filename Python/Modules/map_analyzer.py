import math
import typing
from typing import List

from sc2.game_info import Ramp
from sc2.pixel_map import PixelMap
from sc2.position import Point2

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

class MapAnalyzer:
    bot: 'MyBot'
    grid: PixelMap
    ramps: List[Ramp]

    def __init__(self, bot: 'MyBot'):
        self.bot = bot
        self.placements = {}

    def setup_grid(self):
        self.grid = self.bot.game_info.placement_grid.copy()
        self.ramps = self.bot.game_info.map_ramps.copy()

        mineral_positions: List[Point2] = [mineral.position for mineral in self.bot.mineral_field]
        gas_positions: List[Point2] = [gas.position for gas in self.bot.vespene_geyser]
        base_locations: List[Point2] = [el.position for el in self.bot.information_manager.expansion_locations]

        for base in base_locations:
            x = math.floor(base[0] - 2)
            y = math.floor(base[1] - 2)
            self.null_out_map(x, y, 5, 5)

        for m in mineral_positions:
            x = math.floor(m[0]-1)
            y = math.floor(m[1])
            self.null_out_map(x, y, 2, 1)

        for g in gas_positions:
            x = math.floor(g[0]-1)
            y = math.floor(g[1]-1)
            self.null_out_map(x, y, 3, 3)

        # for each base, null out the area between the base and the mineral line

        # 0,0 0,1 0,2
        # 1,0 1,1 1,2
        # 2,0 2,1 2,2

        # 0,2 1,2 2,2
        # 0,1 1,1 2,1
        # 0,0 1,1 2,2

        asd: PixelMap = self.grid.copy()
        #     rotate
        for x in range(self.grid.width):
            for y in range(self.grid.height):
                asd.__setitem__((x, asd.width-1-y), self.grid.__getitem__((x, y)))
        asd.print()

    def null_out_map(self, start_x, start_y, width, height):
        for i in range(width):
            for j in range(height):
                self.grid.__setitem__((start_x+i, start_y+j), 0)

    def find_placements(self):
        self.setup_grid()
        self.find_supply()
        self.find_tech()
        self.find_production()

    def find_supply(self):
        # generally behind the mineral line
        # otherwise near the base
        pass

    def find_tech(self):
        # generally behind the minara line/gas buildings
        # otherwise near the base
        pass

    def find_production(self):
        # generally further away from the base
        pass
