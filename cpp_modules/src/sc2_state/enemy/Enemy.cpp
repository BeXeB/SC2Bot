//
// Created by marco on 06/05/2025.
//
#include "Enemy.h"

#include <iostream>

EnemyAction Enemy::generateEnemyAction() {
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
    // Specifies how many times the enemy will do nothing
    constexpr double noneAction = 60 - buildUnitAction - attackAction - groundPowerIncrease - airPowerIncrease -
                                  groundProductionIncrease - airProductionIncrease;

    const auto actionWeights = {
        noneAction, buildUnitAction, attackAction, groundPowerIncrease, airPowerIncrease,
        groundProductionIncrease, airProductionIncrease
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
        default:
            return EnemyAction::none;
    }
}

EnemyAction Enemy::takeAction(const int currentTime, std::optional<EnemyAction> action) {
    if (!action) {
        action = generateEnemyAction();
    }
    switch (action.value()) {
        // case Action::addEnemyUnit:
        //     if (currentTime < 90) {
        //         break;
        //     }
        //     break;
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
            addUnits();
        case EnemyAction::addEnemyProduction:
            addProductionBuilding(currentTime);
        case EnemyAction::none:
            break;
        default:
            throw std::invalid_argument("invalid action");
    }
    return action.value();
}

void Enemy::addProductionBuilding(const int currentTime) {
    std::vector<ProductionBuildingType> availableBuildings = {};
    for (const auto &[type, building]: productionBuildings) {
        if (building.timeRequirement > currentTime)
            continue;

        auto requiredBuilding = building.buildingRequirement;
        if (requiredBuilding!= ProductionBuildingType::None && productionBuildings[requiredBuilding].amount < 1 ) {
            continue;
        }

        availableBuildings.emplace_back(type);
    }
    const auto building = randomChoice(availableBuildings);

    productionBuildings[building].amount += 0.1;
}

void Enemy::addUnits() {
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
    const auto building = unitBuildings[unit];
    const auto buildingAmount = productionBuildings[building].amount;

    units[unit] += std::floor(buildingAmount);
}

// int main() {
//     std::cout << "Hello, World!\n";
//     auto enemy = Enemy(EnemyRace::Terran, 1);
//
//
//     auto initialProduction = enemy.productionBuildings;
//     // enemy.takeAction(500, EnemyAction::addEnemyProduction);
//     //
//     //
//     // bool productionIncreased = false;
//     // for (auto [type,building]: initialProduction) {
//     //     productionIncreased = enemy.productionBuildings[type].amount > building.amount;
//     //     if (productionIncreased) {
//     //         break;
//     //     }
//     // }
//     //
//     enemy.takeAction(500);
//     enemy.takeAction(500);
//
//     auto enemy2 = enemy;
//
//     for (int i = 0; i < 50; i++) {
//         auto action = enemy.takeAction(500);
//         auto action2 = enemy2.takeAction(500);
//
//     }
// }
