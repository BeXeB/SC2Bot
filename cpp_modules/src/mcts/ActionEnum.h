//
// Created by marco on 07/11/2024.
//

#ifndef ACTIONENUM_H
#define ACTIONENUM_H
#include <string>

enum class Action {
    none,
    buildWorker,
    buildHouse,
    buildBase,
    buildVespeneCollector,
    buildMarine,
    buildBarracks,
    buildFactory,
    buildTank,
    // buildFactoryTechLab,
    buildStarPort,
    buildViking,
    attackPlayer,
    addEnemyUnit,
    addEnemyGroundPower,
    addEnemyAirPower
};

inline std::string actionToString(Action action) {
    std::string actionString;
    switch (action) {
        case Action::none:
            actionString = std::string("none");
            break;
        case Action::buildWorker:
            actionString = std::string("buildWorker");
            break;
        case Action::buildHouse:
            actionString = std::string("buildHouse");
            break;
        case Action::buildBase:
            actionString = std::string("buildBase");
            break;
        case Action::buildVespeneCollector:
            actionString = std::string("buildVespeneCollector");
            break;
        case Action::buildMarine:
            actionString = std::string("buildMarine");
            break;
        case Action::buildBarracks:
            actionString = std::string("buildBarracks");
            break;
        case Action::buildFactory:
            actionString = std::string("buildFactory");
            break;
        // case Action::buildFactoryTechLab:
        //     actionString = std::string("buildFactoryTechLab");
        //     break;
        case Action::buildStarPort:
            actionString = std::string("buildStarPort");
            break;
        case Action::buildViking:
            actionString = std::string("buildViking");
            break;
        case Action::buildTank:
            actionString = std::string("buildTank");
            break;
        case Action::attackPlayer:
            actionString = std::string("attackPlayer");
            break;
        case Action::addEnemyUnit:
            actionString = std::string("addEnemyUnit");
            break;
        default:
            throw std::runtime_error("Could not convert action to string");
    }

    return actionString;
}

inline std::ostream &operator<<(std::ostream &os, const Action &action) {
    switch (action) {
        case Action::none:
            os << std::string("none");
            break;
        case Action::buildWorker:
            os << std::string("buildWorker");
            break;
        case Action::buildHouse:
            os << std::string("buildHouse");
            break;
        case Action::buildBase:
            os << std::string("buildBase");
            break;
        case Action::buildVespeneCollector:
            os << std::string("buildVespeneCollector");
            break;
        case Action::buildMarine:
            os << std::string("buildMarine");
            break;
        case Action::buildBarracks:
            os << std::string("buildBarracks");
            break;
        case Action::buildFactory:
            os << std::string("buildFactory");
            break;
        // case Action::buildFactoryTechLab:
        //     os << std::string("buildFactoryTechLab");
        //     break;
        case Action::buildStarPort:
            os << std::string("buildStarPort");
            break;
        case Action::buildViking:
            os << std::string("buildViking");
            break;
        case Action::buildTank:
            os << std::string("buildTank");
            break;
        case Action::attackPlayer:
            os << std::string("attackPlayer");
            break;
        case Action::addEnemyUnit:
            os << std::string("addEnemyUnit");
            break;
        default:
            throw std::runtime_error("Could not convert action to string");
    }
    return os;
}

#endif //ACTIONENUM_H
