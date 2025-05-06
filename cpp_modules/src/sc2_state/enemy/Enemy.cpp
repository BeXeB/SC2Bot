//
// Created by marco on 06/05/2025.
//
#include "Enemy.h"
#include "ActionEnum.h"

Action Enemy::generateEnemyAction() {
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
            return Action::addEnemyUnit;
        case 2:
            return Action::attackPlayer;
        case 3:
            return Action::addEnemyGroundPower;
        case 4:
            return Action::addEnemyAirPower;
        case 5:
            return Action::addEnemyGroundProduction;
        case 6:
            return Action::addEnemyAirProduction;
        default:
            return Action::none;
    }
}

Action Enemy::takeAction(const int currentTime, std::optional<Action> action) {
    if (!action) {
        action = generateEnemyAction();
    }
    switch (action.value()) {
        case Action::addEnemyUnit:
            if (currentTime < 90) {
                break;
            }
            break;
        case Action::attackPlayer:
            break;
        case Action::addEnemyGroundPower:
            if (currentTime < 90) {
                break;
            }
            addEnemyGroundPower();
            break;
        case Action::addEnemyAirPower:
            if (currentTime < 120) {
                break;
            }
            addEnemyAirPower();
            break;
        case Action::addEnemyGroundProduction:
            if (currentTime < 90) {
                break;
            }
            addEnemyGroundProduction();
            break;
        case Action::addEnemyAirProduction:
            if (currentTime < 120) {
                break;
            }
            addEnemyAirProduction();
            break;
        // case Action::addEnemyUnit:
        // addUnits();
        // case Action::addEnemyProduction:
        // addProductionBuilding();
        case Action::none:
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
        if (productionBuildings[requiredBuilding].amount < 1 ) {
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
    const auto unit = randomChoice(availableUnits);
    const auto building = unitBuildings[unit];
    const auto buildingAmount = productionBuildings[building].amount;

    units[unit] += std::floor(buildingAmount);
}
