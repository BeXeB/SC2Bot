//
// Created by marco on 07/11/2024.
//

#ifndef ACTIONENUM_H
#define ACTIONENUM_H

enum class Action {
    none,
    buildWorker,
    buildHouse,
    buildBase,
    buildVespeneCollector,
};

inline std::ostream &operator<<(std::ostream &os, const Action &action) {
    switch (action) {
        case Action::none:
            os << "none";
            break;
        case Action::buildWorker:
            os << "buildWorker";
            break;
        case Action::buildHouse:
            os << "buildHouse";
            break;
        case Action::buildBase:
            os << "buildBase";
            break;
        case Action::buildVespeneCollector:
            os << "buildVespeneCollector";
            break;
    }
    return os;
}

#endif //ACTIONENUM_H
