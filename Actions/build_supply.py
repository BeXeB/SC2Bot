from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId
from sc2.position import Point2


class SupplyBuilder:
    def __init__(self, bot: BotAI):
        self.bot = bot
        up_or_down = -1 if bot.start_location.y < self.bot.game_info.map_size.x/2 else 1
        start_location = self.bot.start_location
        first_supply_position = Point2((start_location.x + 2  * up_or_down, start_location.y - 6))
        self.possible_supply_positions = []
        for i in range(1, 6):
            for j in range(1, 6):
                self.possible_supply_positions.append(
                    Point2((first_supply_position.x + i * 2 * up_or_down, first_supply_position.y + j * 2))
                )

        print(self.possible_supply_positions)
        print(len(self.possible_supply_positions))

    async def build_supply(self):
        if self.possible_supply_positions:
            for position in self.possible_supply_positions:
                can_place = await self.bot.can_place_single(UnitTypeId.SUPPLYDEPOT, position)
                if can_place:
                    worker = self.bot.select_build_worker(position)
                    if worker:
                        worker.build(UnitTypeId.SUPPLYDEPOT, position)
                        self.possible_supply_positions.remove(position)
                        break