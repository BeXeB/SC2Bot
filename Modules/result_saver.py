from __future__ import annotations
import typing

from sc2.bot_ai import BotAI
from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from testbot import MyBot

from sc2_mcts import *
import math


def save_result(bot: 'MyBot', final_state: State, game_time: float):
    file = open('../result.csv', 'a')
    total_minerals_mined = bot.minerals - 50
    total_minerals_mined += len(bot.gas_buildings) * 75
    total_minerals_mined += (len(bot.workers) - 12) * 50
    total_minerals_mined += len(bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT)) * 100
    total_minerals_mined += (len(bot.townhalls) - 1) * 400
    file.write(f"{str(bot.mcts_settings[0])},"
               f"{str(bot.mcts_settings[1])},"
               f"{str(bot.mcts_settings[2])},"
               f"{str(bot.mcts_settings[3])},"
               f"{str(bot.mcts_settings[4])},"
               f"{str(bot.action_selection)},"
               f"{str(bot.future_action_queue.maxsize)},"
               f"{str(bot.fixed_search_rollouts)},"
               f"{str(game_time)},"
               f"{str(final_state.get_minerals())},"
               f"{str(total_minerals_mined)},"
               f"{str(final_state.get_vespene())},"
               f"{str(bot.supply_workers)},"
               f"{str(len(bot.townhalls))},"
               f"{str(len(bot.gas_buildings))},"
               f"{str(final_state.get_value())}"
               f"\n")
    file.close()

def save_human_result(bot: BotAI, final_state: State, game_time: float):
    file = open('human_result.csv', 'a')
    total_minerals_mined = bot.minerals - 50
    total_minerals_mined += len(bot.gas_buildings) * 75
    total_minerals_mined += (len(bot.workers) - 12) * 50
    total_minerals_mined += len(bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT)) * 100
    total_minerals_mined += (len(bot.townhalls) - 1) * 400
    file.write(f"{str(game_time)},"
               f"{str(final_state.get_minerals())},"
               f"{str(total_minerals_mined)},"
               f"{str(final_state.get_vespene())},"
               f"{str(bot.supply_workers)},"
               f"{str(len(bot.townhalls))},"
               f"{str(len(bot.gas_buildings))},"
               f"{str(final_state.get_value())}"
               f"\n")
    file.close()
