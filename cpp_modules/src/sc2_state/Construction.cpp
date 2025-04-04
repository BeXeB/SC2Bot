﻿#include "Construction.h"
#include "Sc2State.h"
#include "ActionEnum.h"

//
// Created by marco on 09/12/2024.
//
Sc2::Construction::Construction(const int constructionTime, const Action action) {
    _timeLeft = constructionTime;
    _action = action;

    switch (action) {
        case Action::buildWorker:
            _constructionFunction = &State::addWorker;
            break;
        case Action::buildBarracks:
            _constructionFunction = &State::addBarracks;
            break;
        case Action::buildHouse:
            _constructionFunction = &State::addHouse;
            break;
        case Action::buildBase:
            _constructionFunction = &State::addBase;
            break;
        case Action::buildVespeneCollector:
            _constructionFunction = &State::addVespeneCollector;
            break;
        case Action::buildMarine:
            _constructionFunction = &State::addMarine;
            break;
        case Action::attackPlayer:
            throw std::invalid_argument("Action::attackPlayer");
        case Action::addEnemyUnit:
            throw std::invalid_argument("Action::addEnemyUnit");
        case Action::none:
            throw std::invalid_argument("Action::none");
    }
}
