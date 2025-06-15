//
// Created by marco on 06/05/2025.
//
#include "Enemy.h"

#include <iostream>

Sc2::EnemyAction Sc2::Enemy::generateEnemyAction() {
    // Over the span of 60 seconds we assume that the enemy:
    // Specifies how many enemy units will be built
    constexpr double buildUnitAction = 8;
    // Specifies how much ground power the enemy gets per production
    constexpr double groundPowerIncrease = 5;
    // Specifies how much air power the enemy gets per production
    constexpr double airPowerIncrease = 5;
    // Specifies how much ground production the enemy builds
    constexpr double groundProductionIncrease = 3;
    // Specifies how much air production the enemy builds
    constexpr double airProductionIncrease = 3;
    // Specifies how many times the enemy will attack
    constexpr double attackAction = 0.3;
    // Specifies how many times specific production buildings are added
    constexpr double productionIncrease = 5;
    // Specifies how many times the enemy will do nothing
    constexpr double noneAction = 60 - buildUnitAction - attackAction - groundPowerIncrease - airPowerIncrease -
                                  groundProductionIncrease - airProductionIncrease - productionIncrease;

    const auto actionWeights = {
        noneAction, buildUnitAction, attackAction, groundPowerIncrease, airPowerIncrease,
        groundProductionIncrease, airProductionIncrease, productionIncrease
    };
    std::discrete_distribution<int> dist(actionWeights.begin(), actionWeights.end());
    // 0: None, 1: Build unit, 2: Attack, 3: GroundPowerIncrease, 4: AirPowerIncrease, 5: Ground Production, 6: Air Production
    switch (dist(_rng)) {
        case 1:
            return EnemyAction::addEnemyUnit;
        case 2:
            return EnemyAction::attackPlayer;
        case 3:
            return EnemyAction::addEnemyGroundPower;
        case 4:
            return EnemyAction::addEnemyAirPower;
        case 5:
            return EnemyAction::addEnemyGroundProduction;
        case 6:
            return EnemyAction::addEnemyAirProduction;
        case 7:
            return EnemyAction::addEnemyProduction;
        default:
            return EnemyAction::none;
    }
}

Sc2::EnemyAction Sc2::Enemy::takeAction(const int currentTime, std::optional<EnemyAction> action) {
    if (!action) {
        action = generateEnemyAction();
    }
    switch (action.value()) {

        case EnemyAction::attackPlayer:
            break;
        case EnemyAction::addEnemyGroundPower:
            if (currentTime < 90) {
                break;
            }
            addEnemyGroundPower();
            break;
        case EnemyAction::addEnemyAirPower:
            if (currentTime < 120) {
                break;
            }
            addEnemyAirPower();
            break;
        case EnemyAction::addEnemyGroundProduction:
            if (currentTime < 90) {
                break;
            }
            addEnemyGroundProduction();
            break;
        case EnemyAction::addEnemyAirProduction:
            if (currentTime < 120) {
                break;
            }
            addEnemyAirProduction();
            break;
        case EnemyAction::addEnemyUnit:
            if (currentTime >= 90) {
                enemyCombatUnits += 1;
            }
            addUnits();
            break;
        case EnemyAction::addEnemyProduction:
            addProductionBuilding(currentTime);
            break;
        case EnemyAction::none:
            break;
        default:
            throw std::invalid_argument("invalid action");
    }
    return action.value();
}

void Sc2::Enemy::addProductionBuilding(const int currentTime) {
    std::vector<ProductionBuildingType> availableBuildings = {};
    for (const auto &[type, building]: productionBuildings) {
        if (building.timeRequirement > currentTime)
            continue;

        auto requiredBuilding = building.buildingRequirement;
        if (requiredBuilding!= ProductionBuildingType::None &&
            productionBuildings[requiredBuilding].amount < 1 ) {
            continue;
        }

        availableBuildings.emplace_back(type);
    }

    if (availableBuildings.empty()) {
        return;
    }
    const auto building = randomChoice(availableBuildings);

    if (productionBuildings[building].amount < 1) {
        productionBuildings[building].amount = 1;
        return;
    }

    switch (race) {
        case EnemyRace::Protoss:
            productionBuildings[building].amount += 0.05;
            break;
        case EnemyRace::Terran:
            productionBuildings[building].amount += 0.1;
            break;
        case EnemyRace::Zerg:
            productionBuildings[building].amount += 0.15;
            break;
    }
}

void Sc2::Enemy::addUnits() {
    std::vector<EnemyUnitType> availableUnits = {};
    std::unordered_map<EnemyUnitType, ProductionBuildingType> unitBuildings = {};

    for (auto & [buildingType,building]: productionBuildings) {
        if (building.amount >= 1) {
            auto productionList = building.getProductionList();
            for (auto unit: productionList) {
                availableUnits.emplace_back(unit);
                unitBuildings[unit] = buildingType;
            }
        }
    }

    if (availableUnits.empty()) {
        return;
    }
    const auto unit = randomChoice(availableUnits);
    float buildingAmount = 0;
    if (race == EnemyRace::Zerg) {
        buildingAmount = productionBuildings[ProductionBuildingType::Hatchery].amount;
    }else {
        const auto building = unitBuildings[unit];
        buildingAmount = productionBuildings[building].amount;
    }

    units[unit] += std::floor(buildingAmount);
}