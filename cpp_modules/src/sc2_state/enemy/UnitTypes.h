//
// Created by marco on 02/05/2025.
//

#ifndef UNITTYPES_H
#define UNITTYPES_H
#include <string>
#include <unordered_map>

enum class EnemyUnitType {
    COLOSSUS,
    BANELING,
    MOTHERSHIP,
    CHANGELING,
    CHANGELINGMARINESHIELD,
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
    DRONE,
    ZERGLING,
    HYDRALISK,
    MUTALISK,
    ULTRALISK,
    ROACH,
    INFESTOR,
    CORRUPTOR,
    BROODLORD,
    QUEEN,
    OVERSEER,
    ARCHON,
    BROODLING,
    ADEPT,
    HELLIONTANK,
    LOCUSTMP,
    SWARMHOSTMP,
    ORACLE,
    TEMPEST,
    WIDOWMINE,
    VIPER,
    LURKERMP,
    RAVAGER,
    LIBERATOR,
    CYCLONE,
    LOCUSTMPFLYING,
    DISRUPTOR,
    Last
};

inline std::unordered_map<std::string, EnemyUnitType> stringToUnitType = {
    {"enemy:COLOSSUS", EnemyUnitType::COLOSSUS},
    {"enemy:BANELING", EnemyUnitType::BANELING},
    {"enemy:MOTHERSHIP", EnemyUnitType::MOTHERSHIP},
    {"enemy:CHANGELING", EnemyUnitType::CHANGELING},
    {"enemy:CHANGELINGMARINESHIELD", EnemyUnitType::CHANGELINGMARINESHIELD},
    {"enemy:SIEGETANK", EnemyUnitType::SIEGETANK},
    {"enemy:VIKINGFIGHTER", EnemyUnitType::VIKINGFIGHTER},
    {"enemy:SCV", EnemyUnitType::SCV},
    {"enemy:MARINE", EnemyUnitType::MARINE},
    {"enemy:REAPER", EnemyUnitType::REAPER},
    {"enemy:GHOST", EnemyUnitType::GHOST},
    {"enemy:MARAUDER", EnemyUnitType::MARAUDER},
    {"enemy:THOR", EnemyUnitType::THOR},
    {"enemy:HELLION", EnemyUnitType::HELLION},
    {"enemy:MEDIVAC", EnemyUnitType::MEDIVAC},
    {"enemy:BANSHEE", EnemyUnitType::BANSHEE},
    {"enemy:RAVEN", EnemyUnitType::RAVEN},
    {"enemy:BATTLECRUISER", EnemyUnitType::BATTLECRUISER},
    {"enemy:ZEALOT", EnemyUnitType::ZEALOT},
    {"enemy:STALKER", EnemyUnitType::STALKER},
    {"enemy:HIGHTEMPLAR", EnemyUnitType::HIGHTEMPLAR},
    {"enemy:DARKTEMPLAR", EnemyUnitType::DARKTEMPLAR},
    {"enemy:SENTRY", EnemyUnitType::SENTRY},
    {"enemy:PHOENIX", EnemyUnitType::PHOENIX},
    {"enemy:CARRIER", EnemyUnitType::CARRIER},
    {"enemy:VOIDRAY", EnemyUnitType::VOIDRAY},
    {"enemy:WARPPRISM", EnemyUnitType::WARPPRISM},
    {"enemy:OBSERVER", EnemyUnitType::OBSERVER},
    {"enemy:IMMORTAL", EnemyUnitType::IMMORTAL},
    {"enemy:PROBE", EnemyUnitType::PROBE},
    {"enemy:DRONE", EnemyUnitType::DRONE},
    {"enemy:ZERGLING", EnemyUnitType::ZERGLING},
    {"enemy:HYDRALISK", EnemyUnitType::HYDRALISK},
    {"enemy:MUTALISK", EnemyUnitType::MUTALISK},
    {"enemy:ULTRALISK", EnemyUnitType::ULTRALISK},
    {"enemy:ROACH", EnemyUnitType::ROACH},
    {"enemy:INFESTOR", EnemyUnitType::INFESTOR},
    {"enemy:CORRUPTOR", EnemyUnitType::CORRUPTOR},
    {"enemy:BROODLORD", EnemyUnitType::BROODLORD},
    {"enemy:QUEEN", EnemyUnitType::QUEEN},
    {"enemy:OVERSEER", EnemyUnitType::OVERSEER},
    {"enemy:ARCHON", EnemyUnitType::ARCHON},
    {"enemy:BROODLING", EnemyUnitType::BROODLING},
    {"enemy:ADEPT", EnemyUnitType::ADEPT},
    {"enemy:HELLIONTANK", EnemyUnitType::HELLIONTANK},
    {"enemy:LOCUSTMP", EnemyUnitType::LOCUSTMP},
    {"enemy:SWARMHOSTMP", EnemyUnitType::SWARMHOSTMP},
    {"enemy:ORACLE", EnemyUnitType::ORACLE},
    {"enemy:TEMPEST", EnemyUnitType::TEMPEST},
    {"enemy:WIDOWMINE", EnemyUnitType::WIDOWMINE},
    {"enemy:VIPER", EnemyUnitType::VIPER},
    {"enemy:LURKERMP", EnemyUnitType::LURKERMP},
    {"enemy:RAVAGER", EnemyUnitType::RAVAGER},
    {"enemy:LIBERATOR", EnemyUnitType::LIBERATOR},
    {"enemy:CYCLONE", EnemyUnitType::CYCLONE},
    {"enemy:LOCUSTMPFLYING", EnemyUnitType::LOCUSTMPFLYING},
    {"enemy:DISRUPTOR", EnemyUnitType::DISRUPTOR},
};

static std::map<EnemyUnitType, int> convertToEnum(const std::map<std::string, int> & unitStrings) {
    std::map<EnemyUnitType, int>unitMap = {};
    // Initialize all units to 0
    for (int i = static_cast<int>(EnemyUnitType::COLOSSUS); i != static_cast<int>(EnemyUnitType::Last); i++ ) {
        unitMap[static_cast<EnemyUnitType>(i)] = 0;
    }

    for (auto [str, amount]: unitStrings) {
        auto type = stringToUnitType[str];
        unitMap[type] = amount;
    }
    return unitMap;
};
#endif //UNITTYPES_H
