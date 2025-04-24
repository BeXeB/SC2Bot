from __future__ import annotations
import typing

if typing.TYPE_CHECKING:
    from Python.testbot import MyBot

import math

from sc2.ids.ability_id import AbilityId
from sc2.ids.unit_typeid import UnitTypeId

from sc2_mcts import *

def get_bases(bot: 'MyBot') -> list[Base]:
    bases = []
    for townhall in bot.townhalls.ready:
        vespene_collectors = bot.structures.filter(lambda s: s.type_id == UnitTypeId.REFINERY).closer_than(10, townhall)
        base = Base(
            # id=townhall.tag,
            id=1,
            mineral_fields=len(bot.mineral_field.closer_than(10, townhall)),
            vespene_geysers=len(bot.vespene_geyser.closer_than(10, townhall)),
            vespene_collectors=len(vespene_collectors.filter(lambda s: s.build_progress >= 1)),
        )
        bases.append(base)
    return bases

def get_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    constructions.extend(get_pending_house_constructions(bot))
    constructions.extend(get_pending_base_constructions(bot))
    constructions.extend(get_pending_vespene_constructions(bot))
    constructions.extend(get_pending_barracks_constructions(bot))
    constructions.extend(get_pending_factory_constructions(bot))
    constructions.extend(get_pending_starport_constructions(bot))

    for str in bot.structures.not_ready:
        # Calculate time left based on the build progress
        time_left = math.floor((1-str.build_progress) * bot.game_data.units[str.type_id.value].cost.time / 22.4)
        match str.type_id:
            case UnitTypeId.SUPPLYDEPOT:
                action = Action.build_house
            case UnitTypeId.COMMANDCENTER:
                action = Action.build_base
            case UnitTypeId.REFINERY:
                action = Action.build_vespene_collector
            case UnitTypeId.BARRACKS:
                action = Action.build_barracks
            case UnitTypeId.FACTORY:
                action = Action.build_factory
            case UnitTypeId.STARPORT:
                action = Action.build_starport
            case UnitTypeId.FACTORYTECHLAB:
                action = Action.build_factory
            case _:
                raise ValueError(f"Unknown structure type {str.type_id}")
        construction = Construction(
            time_left=time_left,
            action=action
        )
        constructions.append(construction)

    constructions.extend(get_worker_constructions(bot))
    constructions.extend(get_marine_constructions(bot))
    constructions.extend(get_tank_constructions(bot))
    constructions.extend(get_viking_constructions(bot))
    return constructions


def get_marine_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for barracks in bot.structures(UnitTypeId.BARRACKS):
        if barracks.orders and barracks.orders[0].ability.id == AbilityId.BARRACKSTRAIN_MARINE:
            construction = Construction(
                time_left=math.ceil((1 - barracks.orders[0].progress) * bot.information_manager.build_times[UnitTypeId.MARINE]),
                action=Action.build_marine
            )
            constructions.append(construction)
    return constructions

def get_tank_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for factory in bot.structures(UnitTypeId.FACTORY):
        if factory.has_techlab and factory.orders and factory.orders[0].ability.id == AbilityId.FACTORYTRAIN_SIEGETANK:
            construction = Construction(
                time_left=math.ceil((1 - factory.orders[0].progress) * bot.information_manager.build_times[UnitTypeId.SIEGETANK]),
                action=Action.build_tank
            )
            constructions.append(construction)
    return constructions

def get_viking_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for starport in bot.structures(UnitTypeId.STARPORT):
        if starport.orders and starport.orders[0].ability.id == AbilityId.STARPORTTRAIN_VIKINGFIGHTER:
            construction = Construction(
                time_left=math.ceil((1 - starport.orders[0].progress) * bot.information_manager.build_times[UnitTypeId.VIKING]),
                action=Action.build_viking
            )
            constructions.append(construction)
    return constructions

def get_worker_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for th in bot.townhalls:
        if th.orders and th.orders[0].ability.id == AbilityId.COMMANDCENTERTRAIN_SCV:
            construction = Construction(
                time_left=math.ceil((1 - th.orders[0].progress) * bot.information_manager.build_times[UnitTypeId.SCV]),
                action=Action.build_worker
            )
            constructions.append(construction)
    return constructions


def get_pending_barracks_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for i in range(int(bot.worker_en_route_to_build(UnitTypeId.BARRACKS))):
        construction = Construction(
            time_left=math.ceil(bot.information_manager.build_times[UnitTypeId.BARRACKS]),
            action=Action.build_barracks
        )
        constructions.append(construction)
    return constructions

def get_pending_factory_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for i in range(int(bot.worker_en_route_to_build(UnitTypeId.FACTORY))):
        construction = Construction(
            time_left=math.ceil(bot.information_manager.build_times[UnitTypeId.FACTORY]),
            action=Action.build_factory
        )
        constructions.append(construction)
    return constructions

def get_pending_starport_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for i in range(int(bot.worker_en_route_to_build(UnitTypeId.STARPORT))):
        construction = Construction(
            time_left=math.ceil(bot.information_manager.build_times[UnitTypeId.STARPORT]),
            action=Action.build_starport
        )
        constructions.append(construction)
    return constructions

def get_pending_vespene_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for i in range(int(bot.worker_en_route_to_build(UnitTypeId.REFINERY))):
        construction = Construction(
            time_left=math.ceil(bot.information_manager.build_times[UnitTypeId.REFINERY]),
            action=Action.build_vespene_collector
        )
        constructions.append(construction)
    return constructions


def get_pending_base_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for i in range(int(bot.worker_en_route_to_build(UnitTypeId.COMMANDCENTER))):
        construction = Construction(
            time_left=math.ceil(bot.information_manager.build_times[UnitTypeId.COMMANDCENTER]),
            action=Action.build_base
        )
        constructions.append(construction)
    return constructions

def get_pending_house_constructions(bot: 'MyBot') -> list[Construction]:
    constructions = []
    for i in range(int(bot.worker_en_route_to_build(UnitTypeId.SUPPLYDEPOT))):
        construction = Construction(
            time_left=math.ceil(bot.information_manager.build_times[UnitTypeId.SUPPLYDEPOT]),
            action=Action.build_house
        )
        constructions.append(construction)
    return constructions


def translate_state(bot: 'MyBot') -> State:
    bases = get_bases(bot)
    constructions = get_constructions(bot)
    enemy_combat_units = bot.enemy_units.exclude_type(bot.information_manager.units_to_ignore_for_army).amount
    marines = bot.units.filter(lambda u: u.type_id == UnitTypeId.MARINE).amount
    tanks = bot.units.filter(lambda u: u.type_id in { UnitTypeId.SIEGETANK, UnitTypeId.SIEGETANKSIEGED}).amount
    vikings = bot.units.filter(lambda u: u.type_id in { UnitTypeId.VIKING, UnitTypeId.VIKINGFIGHTER, UnitTypeId.VIKINGASSAULT }).amount
    enemy = Enemy(0,0,1,0)
    state = state_builder(
        minerals=bot.minerals,
        vespene=bot.vespene,
        worker_population=int(bot.supply_workers),
        marine_population=marines,
        tank_population=tanks,
        viking_population=vikings,
        incoming_workers=math.floor(bot.already_pending(UnitTypeId.SCV)),
        incoming_marines=math.floor(bot.already_pending(UnitTypeId.MARINE)),
        incoming_tanks=math.floor(bot.already_pending(UnitTypeId.SIEGETANK)),
        incoming_vikings=math.floor(bot.already_pending(UnitTypeId.VIKINGFIGHTER)),
        population_limit=math.floor(bot.supply_cap),
        bases=bases,
        barracks_amount=bot.structures(UnitTypeId.BARRACKS).ready.amount,
        factory_amount=bot.structures(UnitTypeId.FACTORY).ready.amount,
        starport_amount=bot.structures(UnitTypeId.STARPORT).ready.amount,
        constructions=constructions,
        occupied_worker_timers=[math.ceil(time) for time in bot.busy_workers.values()],
        current_time=math.floor(bot.time),
        end_time = math.floor(bot.time)+bot.time_limit,
        enemy_combat_units=enemy_combat_units,
        has_house = bot.tech_requirement_progress(UnitTypeId.BARRACKS) >= 1,
        incoming_house = math.floor(bot.already_pending(UnitTypeId.SUPPLYDEPOT)) > 0,
        incoming_barracks=math.floor(bot.already_pending(UnitTypeId.BARRACKS)) > 0,
        incoming_factory=math.floor(bot.already_pending(UnitTypeId.FACTORY)),
        incoming_bases = math.floor(bot.already_pending(UnitTypeId.COMMANDCENTER)),
        max_bases = 17,
        enemy=enemy
    )
    return state
