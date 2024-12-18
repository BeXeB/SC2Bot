from __future__ import annotations
import typing

from sc2.ids.unit_typeid import UnitTypeId

if typing.TYPE_CHECKING:
    from testbot import MyBot

from sc2_mcts import *
import math


def save_result(bot: 'MyBot', final_state: State, time: int):
    # list of actions taken?
    # timestamp of actions?

    file = open('result.txt', 'a')
    file.write("Mcts Settings: {\n")
    file.write("\tSeed: " + str(bot.mcts_settings[0]) + "\n")
    file.write("\tRollout End Time: " + str(bot.mcts_settings[1]) + "\n")
    file.write("\tExploration: " + str(bot.mcts_settings[2]) + "\n")
    file.write("\tValue Heuristics: " + str(bot.mcts_settings[3]) + "\n")
    file.write("\tRollout Heuristics: " + str(bot.mcts_settings[4]) + "\n")
    file.write("}\n")
    file.write("Time: " + str(time) + "\n")
    file.write(final_state.to_string())
    file.write("Number of gas buildings: " + str(len(bot.gas_buildings.ready)) + "\n")
    total_minerals_mined = bot.minerals - 50
    total_minerals_mined += len(bot.gas_buildings) * 75
    total_minerals_mined += (len(bot.workers) - 12) * 50
    total_minerals_mined += len(bot.structures.filter(lambda s: s.type_id == UnitTypeId.SUPPLYDEPOT)) * 100
    total_minerals_mined += (len(bot.townhalls) - 1) * 400
    file.write("Total Minerals Mined: " + str(total_minerals_mined) + "\n")
    file.write("Actions Taken: {\n")
    for time, action in bot.actions_taken.items():
        file.write("\tAction: " + str(action) + ", At Iteration: " + str(time) + "\n")
    file.write("}\n")
    file.close()