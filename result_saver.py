from __future__ import annotations
import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from testbot import MyBot

from sc2_mcts import *
import math


def save_result(bot: 'MyBot', final_state: State, time: float):
    file = open('result.csv', 'a')
    total_minerals_mined = bot.minerals - 50
    total_minerals_mined += len(bot.gas_buildings) * 75
    total_minerals_mined += (len(bot.workers) - 12) * 50
    total_minerals_mined += len(bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT)) * 100
    total_minerals_mined += (len(bot.townhalls) - 1) * 400
    actions = ""
    times = ""
    for time, action in bot.actions_taken.items():
        actions += f"{str(action)};"
        times += f"{str(time)};"
    actions = actions[:-1]
    times = times[:-1]
    file.write(f"{str(bot.mcts_settings[0])},"
               f"{str(bot.mcts_settings[1])},"
               f"{str(bot.mcts_settings[2])},"
               f"{str(bot.mcts_settings[3])},"
               f"{str(bot.mcts_settings[4])},"
               f"{str(time)},"
               f"{str(final_state.get_minerals())},"
               f"{str(total_minerals_mined)},"
               f"{str(final_state.get_vespene())},"
               f"{str(final_state.get_population())},"
               f"{str(len(bot.townhalls))},"
               f"{str(len(bot.gas_buildings))},"
               f"{actions},"
               f"{times}\n")
    file.close()