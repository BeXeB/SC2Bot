import math
import typing
from typing import List, Optional

from Python.Modules.information_manager import PlacementType
from sc2.units import Units

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

    def __placement_position_from_type_and_grid_corner(self, corner: (int, int), type: PlacementType) -> Optional[Point2]:
        match type:
            case PlacementType.SUPPLY:
                return Point2((corner[0] + 1, corner[1] + 1))
            case PlacementType.PRODUCTION:
                return Point2((corner[0] + 1.5, corner[1] + 1.5))
            case PlacementType.TECH:
                return Point2((corner[0] + 1.5, corner[1] + 1.5))

    def __set_grid(self, start_x: int, start_y: int, end_x: int, end_y: int, value: int = 0) -> None:
        if end_x < start_x:
            temp = start_x
            start_x = end_x
            end_x = temp
        if end_y < start_y:
            temp = start_y
            start_y = end_y
            end_y = temp
        for i in range(end_x - start_x):
            for j in range(end_y - start_y):
                self.grid.__setitem__((start_x + i, start_y + j), value)

    def __check_buildable(self, corner: (int, int), size: (int, int)) -> bool:
        for x in range(corner[0], corner[0] + size[0] + 1):
            for y in range(corner[1], corner[1] + size[1] + 1):
                if not self.grid.is_set((x,y)):
                    return False
        return True

    def setup_grid(self):
        self.grid = self.bot.game_info.placement_grid.copy()
        self.ramps = self.bot.game_info.map_ramps.copy()

        mineral_positions: List[Point2] = [mineral.position for mineral in self.bot.mineral_field]
        gas_positions: List[Point2] = [gas.position for gas in self.bot.vespene_geyser]
        base_locations: List[Point2] = [el.position for el in self.bot.information_manager.expansion_locations]

        for base in base_locations:
            x = math.floor(base[0] - 2)
            y = math.floor(base[1] - 2)
            self.__set_grid(x, y, x + 5, y + 5)
            minerals: Units = self.bot.mineral_field.filter(lambda mf: mf.distance_to(base) < 15)
            geysers: Units = self.bot.vespene_geyser.filter(lambda m: m.distance_to(base) < 15)
            for unit in minerals + geysers:
                unit_x = math.floor(unit.position[0])
                unit_y = math.floor(unit.position[1])
                self.__set_grid(x + 2, y + 2, unit_x, unit_y)

        for m in mineral_positions:
            x = math.floor(m[0] - 1)
            y = math.floor(m[1])
            self.__set_grid(x, y, x + 2, y + 1)

        for g in gas_positions:
            x = math.floor(g[0] - 1)
            y = math.floor(g[1] - 1)
            self.__set_grid(x, y, x + 3, y + 3)

    def find_placement(self, type: PlacementType, near: Optional[Point2]) -> Optional[Point2]:
        size = self.bot.information_manager.placement_type_to_size[type]
        if near:
            pass
        corner: Optional[(int, int)] = None
        for x in range(self.grid.width):
            for y in range(self.grid.height):
                if self.__check_buildable((x,y), size):
                    corner = (x, y)
                    break
            if corner:
                break

        self.__set_grid(corner[0], corner[1], corner[0] + size[0], corner[1] + size[1])

        return self.__placement_position_from_type_and_grid_corner(corner, type)

    def make_location_buildable(self, location: Point2, type: Optional[PlacementType] = None) -> None:
        x = math.floor(location[0])
        y = math.floor(location[1])

        if not type:
            self.grid.__setitem__((x, y), 1)

        # TODO: The grid stays unavailable if a worker dies on the way to build

        x -= 1
        y -= 1
        size = self.bot.information_manager.placement_type_to_size[type]
        self.__set_grid(x, y, x + size[0], y + size[1], 1)

    def print(self, wide: bool = False):
        for y in range(self.grid.height):
            for x in range(self.grid.width):
                print("#" if self.grid.is_set((x, self.grid.height - y - 1)) else " ", end=(" " if wide else ""))
            print("")

    # These will be needed for the precalculated version
    # def find_placements(self):
    #     self.setup_grid()
    #     self.__find_supply()
    #     self.__find_tech()
    #     self.__find_production()

    # def __find_supply(self):
    #     # generally behind the mineral line
    #     # otherwise near the base
    #     pass
    #
    # def __find_tech(self):
    #     # generally behind the minara line/gas buildings
    #     # otherwise near the base
    #     pass
    #
    # def __find_production(self):
    #     # generally further away from the base
    #     pass


