//
// Created by marco on 02/05/2025.
//

#ifndef PRODUCTIONBUILDINGS_H
#define PRODUCTIONBUILDINGS_H
#include <vector>

#include "UnitTypes.h"

enum class ProductionBuildingType {
    Barracks,
    Factory,
    Starport,
    None,
};

struct ProductionBuilding {
    float amount = 0;
    int timeRequirement = 0;
    ProductionBuildingType buildingRequirement = ProductionBuildingType::None;

    [[nodiscard]] const std::vector<EnemyUnitType>& getProductionList() const {return productionList;}

    ProductionBuilding(const float amount, const int timeRequirement, const ProductionBuildingType buildRequirement, std::vector<EnemyUnitType> const &productionList):
        amount(amount),
        timeRequirement(timeRequirement),
        buildingRequirement(buildRequirement),
        productionList(productionList) {}

    ProductionBuilding(const ProductionBuilding& productionBuilding) {
        amount = productionBuilding.amount;
        productionList = productionBuilding.productionList;
        timeRequirement = productionBuilding.timeRequirement;
        buildingRequirement = productionBuilding.buildingRequirement;
    };

    ProductionBuilding() = default;
private:
    std::vector<EnemyUnitType> productionList;
};

struct BarracksProductionBuilding : public ProductionBuilding {
    explicit BarracksProductionBuilding(const float amount) : ProductionBuilding(amount, 130, ProductionBuildingType::None,  std::vector<EnemyUnitType> {
                                                                                      EnemyUnitType::MARINE,
                                                                                      EnemyUnitType::MARAUDER,
                                                                                      EnemyUnitType::REAPER,
                                                                                      EnemyUnitType::GHOST
                                                                                  })
    {
    }
};

struct FactoryProductionBuilding : public ProductionBuilding {
    explicit FactoryProductionBuilding(const float amount) : ProductionBuilding(amount, 0, ProductionBuildingType::Barracks ,std::vector<EnemyUnitType> {
                                                                                      EnemyUnitType::HELLION,
                                                                                      EnemyUnitType::SIEGETANK,
                                                                                      EnemyUnitType::THOR,
                                                                                      EnemyUnitType::WIDOWMINE,
                                                                                      EnemyUnitType::CYCLONE
                                                                                  })
    {
    }
};

struct StarportProductionBuilding : public ProductionBuilding {
    explicit StarportProductionBuilding(const float amount) : ProductionBuilding(amount, 0, ProductionBuildingType::Factory ,std::vector<EnemyUnitType> {
                                                                                      EnemyUnitType::VIKINGFIGHTER,
                                                                                      EnemyUnitType::MEDIVAC,
                                                                                      EnemyUnitType::LIBERATOR,
                                                                                      EnemyUnitType::RAVEN,
                                                                                      EnemyUnitType::BANSHEE,
                                                                                      EnemyUnitType::BATTLECRUISER
                                                                                  })
    {
    }
};
#endif //PRODUCTIONBUILDINGS_H
