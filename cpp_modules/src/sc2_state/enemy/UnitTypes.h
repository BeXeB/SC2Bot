//
// Created by marco on 02/05/2025.
//

#ifndef UNITTYPES_H
#define UNITTYPES_H
#include <string>
#include <unordered_map>

enum class EnemyUnitType {
    COLOSSUS,
    MOTHERSHIP,
    SIEGETANK,
    VIKINGFIGHTER,
    SCV,
    MARINE,
    REAPER,
    GHOST,
    MARAUDER,
    THOR,
    HELLION,
    MEDIVAC,
    BANSHEE,
    RAVEN,
    BATTLECRUISER,
    ZEALOT,
    STALKER,
    HIGHTEMPLAR,
    DARKTEMPLAR,
    SENTRY,
    PHOENIX,
    CARRIER,
    VOIDRAY,
    WARPPRISM,
    OBSERVER,
    IMMORTAL,
    PROBE,
    ARCHON,
    ADEPT,
    HELLIONTANK,
    ORACLE,
    TEMPEST,
    WIDOWMINE,
    LIBERATOR,
    CYCLONE,
    DISRUPTOR,
};

inline std::pmr::unordered_map<EnemyUnitType, std::string> UnitTypeToString = {
    {EnemyUnitType::COLOSSUS, "enemy:COLOSSUS"},
    {EnemyUnitType::MOTHERSHIP, "enemy:MOTHERSHIP"},
    {EnemyUnitType::SIEGETANK, "enemy:SIEGETANK"},
    {EnemyUnitType::VIKINGFIGHTER, "enemy:VIKINGFIGHTER"},
    {EnemyUnitType::SCV, "enemy:SCV"},
    {EnemyUnitType::MARINE, "enemy:MARINE"},
    {EnemyUnitType::REAPER, "enemy:REAPER"},
    {EnemyUnitType::GHOST, "enemy:GHOST"},
    {EnemyUnitType::MARAUDER, "enemy:MARAUDER"},
    {EnemyUnitType::THOR, "enemy:THOR"},
    {EnemyUnitType::HELLION, "enemy:HELLION"},
    {EnemyUnitType::MEDIVAC, "enemy:MEDIVAC"},
    {EnemyUnitType::BANSHEE, "enemy:BANSHEE"},
    {EnemyUnitType::RAVEN, "enemy:RAVEN"},
    {EnemyUnitType::BATTLECRUISER, "enemy:BATTLECRUISER"},
    {EnemyUnitType::ZEALOT, "enemy:ZEALOT"},
    {EnemyUnitType::STALKER, "enemy:STALKER"},
    {EnemyUnitType::HIGHTEMPLAR, "enemy:HIGHTEMPLAR"},
    {EnemyUnitType::DARKTEMPLAR, "enemy:DARKTEMPLAR"},
    {EnemyUnitType::SENTRY, "enemy:SENTRY"},
    {EnemyUnitType::PHOENIX, "enemy:PHOENIX"},
    {EnemyUnitType::CARRIER, "enemy:CARRIER"},
    {EnemyUnitType::VOIDRAY, "enemy:VOIDRAY"},
    {EnemyUnitType::WARPPRISM, "enemy:WARPPRISM"},
    {EnemyUnitType::OBSERVER, "enemy:OBSERVER"},
    {EnemyUnitType::IMMORTAL, "enemy:IMMORTAL"},
    {EnemyUnitType::PROBE, "enemy:PROBE"},
    {EnemyUnitType::ARCHON, "enemy:ARCHON"},
    {EnemyUnitType::ADEPT, "enemy:ADEPT"},
    {EnemyUnitType::HELLIONTANK, "enemy:HELLIONTANK"},
    {EnemyUnitType::ORACLE, "enemy:ORACLE"},
    {EnemyUnitType::TEMPEST, "enemy:TEMPEST"},
    {EnemyUnitType::WIDOWMINE, "enemy:WIDOWMINE"},
    {EnemyUnitType::LIBERATOR, "enemy:LIBERATOR"},
    {EnemyUnitType::CYCLONE, "enemy:CYCLONE"},
    {EnemyUnitType::DISRUPTOR, "enemy:DISRUPTOR"}

};

#endif //UNITTYPES_H
