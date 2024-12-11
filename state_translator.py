from __future__ import annotations

from dataclasses import dataclass
import math
from typing import Optional

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId
from sc2.unit import Unit

from cpp_modules.module_test.module_test import action
from sc2_mcts import *

# from testbot import MyBot


# @dataclass
# class Base:
#     number_of_mineral_fields: int
#     vespene_geysers: int
#     # current number of collectors
#     vespene_collectors: int
#     # collectors under construction
#     incoming_vespene_collectors: int
#
#
# @dataclass
# class Construction:
#     time_left: int
#     unit_type: UnitTypeId
#
# @dataclass
# class State:
#     minerals: int
#     vespene: int
#     # worker population
#     population: int
#     # workers under construction
#     incoming_population: int
#     population_limit: int
#     bases: list[Base]
#     constructions: list[Construction]
#     # workers that are currently busy, and the time left
#     busy_workers: list[int]

def get_bases(bot: 'MyBot') -> list[Base]:
    bases = []
    for townhall in bot.townhalls:
        vespene_collectors = bot.structures.filter(lambda s: s.type_id == UnitTypeId.REFINERY).closer_than(10, townhall)
        base = Base(
            # id=townhall.tag,
            id=1,
            mineral_fields=len(bot.mineral_field.closer_than(10, townhall)),
            vespene_geysers=len(bot.vespene_geyser.closer_than(10, townhall)),
            vespene_collectors=len(vespene_collectors.filter(lambda s: s.build_progress >= 1)),
            # incoming_vespene_collectors=len(vespene_collectors.filter(lambda s: s.build_progress < 1))
        )
        bases.append(base)
    return bases

def get_constructions(bot: 'MyBot') -> list[Construction]:
    # TODO: This is a very hacky way to get the pending constructions
    constructions = []
    for i in range(bot.worker_en_route_to_build(UnitTypeId.SUPPLYDEPOT)):
        construction = Construction(
            time_left=math.ceil(bot.SUPPLY_BUILD_TIME_SECONDS),
            action=Action.build_house
        )
        constructions.append(construction)
    for i in range(bot.worker_en_route_to_build(UnitTypeId.COMMANDCENTER)):
        construction = Construction(
            time_left=math.ceil(bot.CC_BUILD_TIME_SECONDS),
            action=Action.build_base
        )
        constructions.append(construction)
    for i in range(bot.worker_en_route_to_build(UnitTypeId.REFINERY)):
        construction = Construction(
            time_left=math.ceil(bot.REFINERY_BUILD_TIME_SECONDS),
            action=Action.build_vespene_collector
        )
        constructions.append(construction)
    for str in bot.structures.not_ready:
        # Calculate time left based on the build progress
        time_left = math.floor(str.build_progress * bot.game_data.units[str.type_id.value]._proto.build_time / 22.4)
        match str.type_id:
            case UnitTypeId.SUPPLYDEPOT:
                action = Action.build_house
            case UnitTypeId.COMMANDCENTER:
                action = Action.build_base
            case UnitTypeId.REFINERY:
                action = Action.build_vespene_collector
            case _:
                raise ValueError(f"Unknown structure type {str.type_id}")
        construction = Construction(
            time_left=time_left,
            action=action
        )
        constructions.append(construction)
    for th in bot.townhalls:
        if th.orders and th.orders[0].ability.id == AbilityId.COMMANDCENTERTRAIN_SCV:
            construction = Construction(
                time_left=math.ceil((1 - th.orders[0].progress) * bot.WORKER_BUILD_TIME_SECONDS),
                action=Action.build_worker
            )
            constructions.append(construction)
    return constructions

def translate_state(bot: 'MyBot') -> State:
    bases = get_bases(bot)
    constructions = get_constructions(bot)
    state = state_builder(
        minerals=bot.minerals,
        vespene=bot.vespene,
        population=bot.workers.amount,
        incoming_population=math.floor(bot.already_pending(UnitTypeId.SCV)),
        population_limit=math.floor(bot.supply_cap),
        bases=bases,
        constructions=constructions,
        occupied_worker_timers=[math.ceil(time) for time in bot.busy_workers.values()]
    )
    return state
