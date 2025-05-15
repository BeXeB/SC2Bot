//
// Created by marco on 02/05/2025.
//

#ifndef PRODUCTIONBUILDINGS_H
#define PRODUCTIONBUILDINGS_H
#include <vector>

#include "UnitTypes.h"

namespace Sc2 {
    enum class ProductionBuildingType {
        Barracks,
        Factory,
        Starport,
        HydraliskDen,
        Hatchery,
        LurkerDen,
        UltraliskCavern,
        SpawningPool,
        RoachWarren,
        Spire,
        BanelingNest,
        InfestationPit,
        Gateway,
        Stargate,
        RoboticsFacility,
        None,
    };

    struct ProductionBuilding {
        float amount = 0;
        int timeRequirement = 0;
        ProductionBuildingType type = ProductionBuildingType::None;
        ProductionBuildingType buildingRequirement = ProductionBuildingType::None;

        [[nodiscard]] const std::vector<EnemyUnitType>& getProductionList() const {return productionList;}

        ProductionBuilding(const ProductionBuildingType type,const float amount, const int timeRequirement,
                           const ProductionBuildingType buildRequirement, std::vector<EnemyUnitType> const &productionList):
            amount(amount),
            timeRequirement(timeRequirement),
            type(type),
            buildingRequirement(buildRequirement),
            productionList(productionList){}

        ProductionBuilding(const ProductionBuilding& productionBuilding) {
            amount = productionBuilding.amount;
            productionList = productionBuilding.productionList;
            timeRequirement = productionBuilding.timeRequirement;
            buildingRequirement = productionBuilding.buildingRequirement;
            type = productionBuilding.type;
        };

        ProductionBuilding() = default;
    private:
        std::vector<EnemyUnitType> productionList;
    };

    struct BarracksProductionBuilding : public ProductionBuilding {
        explicit BarracksProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::Barracks ,
            amount,
            130,
            ProductionBuildingType::None,
            std::vector<EnemyUnitType> {EnemyUnitType::MARINE,
                                        EnemyUnitType::MARAUDER,
                                        EnemyUnitType::REAPER,
                                        EnemyUnitType::GHOST
            })
        {
        }
    };

    struct FactoryProductionBuilding : public ProductionBuilding {
        explicit FactoryProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::Factory ,
            amount,
            0,
            ProductionBuildingType::Barracks ,
            std::vector<EnemyUnitType> {EnemyUnitType::HELLION,
                                        EnemyUnitType::SIEGETANK,
                                        EnemyUnitType::THOR,
                                        EnemyUnitType::WIDOWMINE,
                                        EnemyUnitType::CYCLONE
            })
        {
        }
    };

    struct StarportProductionBuilding : public ProductionBuilding {
        explicit StarportProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::Starport ,
            amount,
            0,
            ProductionBuildingType::Factory,
            std::vector<EnemyUnitType> {EnemyUnitType::VIKINGFIGHTER,
                                        EnemyUnitType::MEDIVAC,
                                        EnemyUnitType::LIBERATOR,
                                        EnemyUnitType::RAVEN,
                                        EnemyUnitType::BANSHEE,
                                        EnemyUnitType::BATTLECRUISER
            })
        {
        }
    };

    struct HatcheryProductionBuilding : public ProductionBuilding {
        explicit HatcheryProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::Hatchery,
            amount,
            0,
            ProductionBuildingType::None,
            std::vector<EnemyUnitType>{}){}
    };
    struct SpawningPoolProductionBuilding : public ProductionBuilding {
        explicit SpawningPoolProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::SpawningPool,
            amount,
            0,
            ProductionBuildingType::None,
            std::vector<EnemyUnitType>{ EnemyUnitType::ZERGLING, EnemyUnitType::QUEEN,})
        {}
    };
    struct BanelingNestProductionBuilding : public ProductionBuilding {
        explicit BanelingNestProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::BanelingNest,
            amount,
            0,
            ProductionBuildingType::SpawningPool,
            std::vector<EnemyUnitType>{EnemyUnitType::BANELING}){}
    };
    struct RoachWarrenProductionBuilding : public ProductionBuilding {
        explicit RoachWarrenProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::RoachWarren,
            amount,
            0,
            ProductionBuildingType::SpawningPool,
            std::vector<EnemyUnitType>{EnemyUnitType::ROACH }){}
    };
    struct HydraliskProductionBuilding : public ProductionBuilding {
        explicit HydraliskProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::HydraliskDen,
            amount,
            0,
            ProductionBuildingType::SpawningPool,
            std::vector<EnemyUnitType>{EnemyUnitType::HYDRALISK}){}
    };
    struct LurkerDenProductionBuilding : public ProductionBuilding {
        explicit LurkerDenProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::RoachWarren,
            amount,
            0,
            ProductionBuildingType::HydraliskDen,
            std::vector<EnemyUnitType>{EnemyUnitType::LURKERMP }){}
    };
    struct InfestationPitProductionBuilding : public ProductionBuilding {
        explicit InfestationPitProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::InfestationPit,
            amount,
            0,
            ProductionBuildingType::SpawningPool,
            std::vector<EnemyUnitType>{EnemyUnitType::INFESTOR,EnemyUnitType::SWARMHOSTMP}){}
    };
    struct SpireProductionBuilding : public ProductionBuilding {
        explicit SpireProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::Spire,
            amount,
            0,
            ProductionBuildingType::InfestationPit,
            std::vector<EnemyUnitType>{EnemyUnitType::MUTALISK,EnemyUnitType::CORRUPTOR, EnemyUnitType::BROODLORD}){}
    };
    struct UltraliskCavernProductionBuilding : public ProductionBuilding {
        explicit UltraliskCavernProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::UltraliskCavern,
            amount,
            0,
            ProductionBuildingType::InfestationPit,
            std::vector<EnemyUnitType>{EnemyUnitType::ULTRALISK}){}
    };

    struct GatewayProductionBuilding : public ProductionBuilding {
        explicit GatewayProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::Gateway,
            amount,
            0,
            ProductionBuildingType::None,
            std::vector<EnemyUnitType>{ EnemyUnitType::ZEALOT,
                                        EnemyUnitType::STALKER,
                                        EnemyUnitType::SENTRY,
                                        EnemyUnitType::ADEPT,
                                        EnemyUnitType::DARKTEMPLAR,
                                        EnemyUnitType::HIGHTEMPLAR}){}
    };
    struct StargateProductionBuilding : public ProductionBuilding {
        explicit StargateProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::Stargate,
            amount,
            0,
            ProductionBuildingType::Gateway,
            std::vector<EnemyUnitType>{ EnemyUnitType::PHOENIX,
                                        EnemyUnitType::VOIDRAY,
                                        EnemyUnitType::ORACLE,
                                        EnemyUnitType::TEMPEST,
                                        EnemyUnitType::CARRIER}){}
    };
    struct RoboticsFacilityProductionBuilding : public ProductionBuilding {
        explicit RoboticsFacilityProductionBuilding(const float amount) : ProductionBuilding(ProductionBuildingType::RoboticsFacility,
            amount,
            0,
            ProductionBuildingType::Gateway,
            std::vector<EnemyUnitType>{ EnemyUnitType::WARPPRISM,
                                        EnemyUnitType::OBSERVER,
                                        EnemyUnitType::IMMORTAL,
                                        EnemyUnitType::COLOSSUS,
                                        EnemyUnitType::DISRUPTOR}){}
    };

    inline std::unordered_map<ProductionBuildingType, ProductionBuilding> convertToProductionBuildings(
        const std::map<ProductionBuildingType, int> & buildingAmounts) {
        std::unordered_map<ProductionBuildingType, ProductionBuilding> buildings;
        for (auto [type, amount]: buildingAmounts) {
            switch (type) {
                case ProductionBuildingType::Barracks:
                    buildings[type] = BarracksProductionBuilding(amount);
                    break;
                case ProductionBuildingType::Factory:
                    buildings[type] = FactoryProductionBuilding(amount);
                    break;
                case ProductionBuildingType::Starport:
                    buildings[type] = StarportProductionBuilding(amount);
                    break;
                case ProductionBuildingType::Hatchery:
                    buildings[type] = HatcheryProductionBuilding(amount);
                case ProductionBuildingType::HydraliskDen:
                    buildings[type] = HydraliskProductionBuilding(amount);
                    break;
                case ProductionBuildingType::LurkerDen:
                    buildings[type] = LurkerDenProductionBuilding(amount);
                    break;
                case ProductionBuildingType::UltraliskCavern:
                    buildings[type] = UltraliskCavernProductionBuilding(amount);
                    break;
                case ProductionBuildingType::SpawningPool:
                    buildings[type] = SpawningPoolProductionBuilding(amount);
                    break;
                case ProductionBuildingType::RoachWarren:
                    buildings[type] = RoachWarrenProductionBuilding(amount);
                    break;
                case ProductionBuildingType::Spire:
                    buildings[type] = SpireProductionBuilding(amount);
                    break;
                case ProductionBuildingType::BanelingNest:
                    buildings[type] = BanelingNestProductionBuilding(amount);
                    break;
                case ProductionBuildingType::InfestationPit:
                    buildings[type] = InfestationPitProductionBuilding(amount);
                    break;
                case ProductionBuildingType::RoboticsFacility:
                    buildings[type] = RoboticsFacilityProductionBuilding(amount);
                    break;
                case ProductionBuildingType::Gateway:
                    buildings[type] = GatewayProductionBuilding(amount);
                    break;
                case ProductionBuildingType::Stargate:
                    buildings[type] = StargateProductionBuilding(amount);
                    break;
                case ProductionBuildingType::None:
                    break;
            }
        }
        return buildings;
    };
}
#endif //PRODUCTIONBUILDINGS_H
