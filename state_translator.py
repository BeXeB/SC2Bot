from __future__ import annotations

from dataclasses import dataclass
import math

from sc2.ids.unit_typeid import UnitTypeId
from sc2.unit import Unit


@dataclass
class Base:
    number_of_mineral_fields: int
    vespene_geysers: int
    # current number of collectors
    vespene_collectors: int
    # collectors under construction
    incoming_vespene_collectors: int


@dataclass
class Construction:
    time_left: int
    unit_type: UnitTypeId

@dataclass
class State:
    minerals: int
    vespene: int
    # worker population
    population: int
    # workers under construction
    incoming_population: int
    population_limit: int
    bases: list[Base]
    constructions: list[Construction]
    # workers that are currently busy, and the time left
    busy_workers: list[int]

def translate_state(bot: 'MyBot') -> State:
    bases = []
    for townhall in bot.townhalls:
        vespene_collectors = bot.structures.filter(lambda s: s.type_id == UnitTypeId.REFINERY).closer_than(10, townhall)
        base = Base(
            number_of_mineral_fields=len(bot.mineral_field.closer_than(10, townhall)),
            vespene_geysers=len(bot.vespene_geyser.closer_than(10, townhall)),
            vespene_collectors=len(vespene_collectors.filter(lambda s: s.build_progress >= 1)),
            incoming_vespene_collectors=len(vespene_collectors.filter(lambda s: s.build_progress < 1))
        )
        bases.append(base)
    # TODO: This is a very hacky way to get the pending constructions
    constructions = []
    for i in range(bot.worker_en_route_to_build(UnitTypeId.SUPPLYDEPOT)):
        construction = Construction(
            time_left=math.ceil(bot.SUPPLY_BUILD_TIME_SECONDS),
            unit_type=UnitTypeId.SUPPLYDEPOT
        )
        constructions.append(construction)
    for i in range(bot.worker_en_route_to_build(UnitTypeId.COMMANDCENTER)):
        construction = Construction(
            time_left=math.ceil(bot.CC_BUILD_TIME_SECONDS),
            unit_type=UnitTypeId.COMMANDCENTER
        )
        constructions.append(construction)
    for i in range(bot.worker_en_route_to_build(UnitTypeId.REFINERY)):
        construction = Construction(
            time_left=math.ceil(bot.REFINERY_BUILD_TIME_SECONDS),
            unit_type=UnitTypeId.REFINERY
        )
        constructions.append(construction)
    for str in bot.structures.not_ready:
        # Calculate time left based on the build progress
        time_left = math.floor(str.build_progress * bot.game_data.units[str.type_id.value]._proto.build_time / 22.4)
        construction = Construction(
            time_left=time_left,
            unit_type=str.type_id
        )
        constructions.append(construction)
    state = State(
        minerals=bot.minerals,
        vespene=bot.vespene,
        population=bot.workers.amount,
        incoming_population=math.floor(bot.already_pending(UnitTypeId.SCV)),
        population_limit=math.floor(bot.supply_cap),
        bases=bases,
        constructions=constructions,
        busy_workers=[time for time in bot.busy_workers.values()]
    )
    return state
