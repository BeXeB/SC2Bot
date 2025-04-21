import copy
import csv
from enum import Enum
from typing import List

from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId

from sc2.unit import Unit

from Python.Modules.army_manager import ArmyManager
from Python.Modules.information_manager import InformationManager
from Python.testbot import MyBot


class MatchupResult(Enum):
    win:int = 1
    tie:int = 0
    loss:int = -1

class Matchup():
    player_units: List[Unit]
    enemy_units: List[Unit]
    result: MatchupResult

    def __init__(self):
        pass

class BattleBot(MyBot):
    information_manager: InformationManager
    army_manager: ArmyManager

    enemy_units_amount: int
    units_amount: int
    game_is_running: bool
    last_unit_death_time: float
    current_matchup: Matchup
    matchups: List[Matchup]

    def __init__(self):
        super().__init__()

        self.enemy_units_amount = 0
        self.current_matchup = Matchup()
        self.matchups = []
        self.units_amount = 0
        self.game_is_running = False
        self.last_unit_death_time = 0


    async def on_start(self):
        self.information_manager = InformationManager(self)
        self.army_manager = ArmyManager(self)

    async def on_step(self, iteration: int) -> None:

        self.army_manager.attack_enemy()

        await self.manage_round_data()

    async def on_unit_destroyed(self, unit_tag: int) -> None:
        await self.information_manager.remove_unit_by_tag(unit_tag)

    async def on_unit_created(self, unit: Unit):
        pass

    async def on_end(self, game_result: MatchupResult):
        fieldnames = ["player:" + ut for ut in UnitTypeId._member_names_]
        fieldnames.extend(["enemy:"+ ut for ut in UnitTypeId._member_names_])
        fieldnames.append("result")

        with open('data/micro_arena.csv', 'w', newline='') as csvfile:
            data : List[dict] = []
            # fieldnames = ['vikings', 'siege_tanks', 'marines', 'enemy_vikings', 'enemy_siege_tanks', 'enemy_marines', 'success']
            for matchup in self.matchups:
                player_unit_dict = self.get_units_for("player", matchup)
                enemy_unit_dict = self.get_units_for("enemy", matchup)
                match_dict = player_unit_dict | enemy_unit_dict
                match_dict.update({"result": matchup.result.value})
                data.append(match_dict)


            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(data)
        pass

    def get_units_for(self, player: str, matchup:Matchup) -> dict:
        res: dict ={}

        for name in UnitTypeId._member_names_:
            res[player + ":" + name] = 0

        for unit in matchup.player_units:
            res[player + ":" + unit.type_id.name] += 1
        return res

    async def manage_round_data(self):
        new_units_amount = len(self.units)
        new_enemy_units_amount = len(self.enemy_units)
        if (new_units_amount != self.units_amount) or (new_enemy_units_amount != self.enemy_units_amount):
            self.last_unit_death_time = self.time

        self.units_amount = new_units_amount
        self.enemy_units_amount = new_enemy_units_amount

        if (not self.game_is_running) and (self.enemy_units_amount > 0 and self.units_amount > 0):
            await self.start_round()
        elif self.game_is_running and self.round_should_end():
            await self.end_round()

    def round_should_end(self):
        return (self.enemy_units_amount <= 0
                or self.units_amount <= 0
                or self.time - self.last_unit_death_time > 60)

    async def start_round(self):
        await self.chat_send("Starting round")
        self.game_is_running = True
        self.current_matchup.enemy_units = self.enemy_units
        self.current_matchup.player_units = self.units

    async def end_round(self):
        await self.chat_send("Ending round")
        self.game_is_running = False
        if self.units_amount == self.enemy_units_amount:
            self.current_matchup.result = MatchupResult.tie
        elif self.units_amount > self.enemy_units_amount:
            self.current_matchup.result = MatchupResult.win
        elif self.enemy_units_amount > self.units_amount:
            self.current_matchup.result = MatchupResult.loss
        self.matchups.append(copy.copy(self.current_matchup))


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