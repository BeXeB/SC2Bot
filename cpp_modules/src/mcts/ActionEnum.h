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
    }
    return os;
}

#endif //ACTIONENUM_H
