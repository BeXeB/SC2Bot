
import csv
from sc2.data import Result
from sc2.bot_ai import BotAI
from sc2.game_state import ChatMessage
from sc2.ids.unit_typeid import UnitTypeId

from sc2.unit import Unit

from Python.Modules.army_manager import ArmyManager
from Python.Modules.information_manager import InformationManager
from Python.testbot import MyBot

class BattleBot(MyBot):
    information_manager: InformationManager
    army_manager: ArmyManager

    def __init__(self):
        super().__init__()
        self.messages: [ChatMessage] = []


    async def on_start(self):
        self.information_manager = InformationManager(self)
        self.army_manager = ArmyManager(self)

    async def on_step(self, iteration: int) -> None:

        self.army_manager.manage_army()
        self.army_manager.check_base_radius()
        self.messages.extend(self.state.chat)
        print(self.time)

    async def on_unit_destroyed(self, unit_tag: int) -> None:
        await self.information_manager.remove_unit_by_tag(unit_tag)

    async def on_unit_created(self, unit: Unit):
        pass

    async def on_end(self, game_result: Result):
        data = [{'vikings': 1, 'siege_tanks': 3, 'marines': 10, 'enemy_vikings': 1, 'enemy_siege_tanks': 3, 'enemy_marines': 10, 'success': 1}]

        with open('data/micro_arena.csv', 'w', newline='') as csvfile:
            # fieldnames = ['vikings', 'siege_tanks', 'marines', 'enemy_vikings', 'enemy_siege_tanks', 'enemy_marines', 'success']
            fieldnames = ['id', 'message']
            data = [{'id':m.player_id,'message':m.message} for m in self.messages]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(data)
        pass
class PeacefulBot(BotAI):
    async def on_step(self, iteration: int) -> None:
        # if iteration == 0:
        #     await self.client.debug_create_unit([[UnitTypeId.MARINE, 10, Point2((self.enemy_start_locations[0].x+10, self.enemy_start_locations[0].y+10)), 2]])
        try:
            if self.can_afford(UnitTypeId.SUPPLYDEPOT) and self.supply_left < 5:
                await self.build(UnitTypeId.SUPPLYDEPOT, near=self.townhalls.random)
            if self.can_afford(UnitTypeId.BARRACKS) and self.structures(UnitTypeId.BARRACKS).amount < 3:
                await self.build(UnitTypeId.BARRACKS, near=self.townhalls.random)
            if self.can_afford(UnitTypeId.MARINE):
                self.train(UnitTypeId.MARINE)
            await self.distribute_workers()
        except:
            return