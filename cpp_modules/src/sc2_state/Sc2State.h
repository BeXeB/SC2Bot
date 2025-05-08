#pragma once
#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <vector>
#include <sstream>

#include "Base.h"
#include "Construction.h"
#include "ActionEnum.h"
#include "UnitPower.h"

namespace Sc2 {
	struct Enemy {
		int groundPower = 0;
		int airPower = 0;
		double groundProduction = 1;
		double airProduction = 0;

		Enemy(const int groundPower, const int airPower, const double groundProduction, const double airProduction) {
			this->groundPower = groundPower;
			this->airPower = airPower;
			this->groundProduction = groundProduction;
			this->airProduction = airProduction;
		}

		Enemy() = default;
	};

	struct StateBuilderParams {
		const int minerals = 0;
		const int vespene = 0;
		const int workerPopulation = 0;
		const int marinePopulation = 0;
		const int tankPopulation = 0;
		const int vikingPopulation = 0;
		const int incomingWorkers = 0;
		const int incomingMarines = 0;
		const int incomingTanks = 0;
		const int incomingVikings = 0;
		const int populationLimit = 0;
		std::vector<Base> &bases;
		const int barracksAmount = 0;
		const int factoryAmount = 0;
		const int starPortAmount = 0;
		std::list<Construction> &constructions;
		std::vector<int> &occupiedWorkerTimers;
		const int currentTime = 0;
		const int endTime = 0;
		const int enemyCombatUnits = 0;
		const bool hasHouse = false;
		const bool incomingHouse = false;
		const bool incomingBarracks = false;
		const int incomingFactory = 0;
		const int incomingBases = 0;
		const int maxBases = 0;
		Enemy enemy;
	};

	enum class ArmyValueFunction {
		MinPower,
		AveragePower,
		ScaledPower,
		MarinePower,
		None
	};

	inline std::ostream &operator<<(std::ostream &os, const ArmyValueFunction &armyValueFunction) {
		switch (armyValueFunction) {
			case ArmyValueFunction::AveragePower:
				os << "AveragePower";
				break;
			case ArmyValueFunction::ScaledPower:
				os << "ScaledPower";
				break;
			case ArmyValueFunction::None:
				os << "None";
				break;
			case ArmyValueFunction::MinPower:
				os << "MinPower";
				break;
			case ArmyValueFunction::MarinePower:
				os << "MarinePower";
				break;
			default:
				os << "Unknown";
		}

		return os;
	}

	class State : public std::enable_shared_from_this<State> {
		ArmyValueFunction _armyValueFunction;
		int END_PROBABILITY_FUNCTION;


		int _minerals = 50;
		int _vespene = 0;
		int _workerPopulation = 12;
		const int MAX_SCOUT_POPULATION = 1;
		int _marinePopulation = 0;
		int _tankPopulation = 0;
		int _vikingPopulation = 0;
		int _incomingWorkers = 0;
		int _incomingMarines = 0;
		int _incomingTanks = 0;
		int _incomingVikings = 0;
		int _incomingVespeneCollectors = 0;
		const int MAX_POPULATION_LIMIT = 200;
		const int MAX_BASES = 17;
		int _populationLimit = 15;
		int _barracksAmount = 0;
		int _factoryAmount = 0;
		int _starPortAmount = 0;
		std::vector<Base> _bases = std::vector{Base()};
		std::list<Construction> _constructions{};
		std::vector<int> _occupiedWorkerTimers{};
		std::mt19937 _rng;

		int _enemyCombatUnits = 0;
		Enemy _enemy;

		const int _endTime;
		int _currentTime = 0;
		bool _hasHouse = false;

		bool _onRollout = false;
		bool _wasAttacked = false;
		bool _incomingHouse = false;
		bool _incomingBarracks = false;
		int _incomingFactory = 0;
		int _incomingBases = 0;

		struct ActionCost {
			int minerals;
			int vespene;
			int buildTime;

			ActionCost(const int minerals, const int vespene, const int buildTime): minerals(minerals),
				vespene(vespene),
				buildTime(buildTime) {
			}
		};

		ActionCost buildWorkerCost = ActionCost(50, 0, 12);
		ActionCost buildBaseCost = ActionCost(400, 0, 71);
		ActionCost buildHouseCost = ActionCost(100, 0, 21);
		ActionCost buildVespeneCollectorCost = ActionCost(75, 0, 21);
		ActionCost buildMarineCost = ActionCost(50, 0, 18);
		ActionCost buildBarracksCost = ActionCost(150, 0, 46);
		ActionCost buildFactoryCost = ActionCost(200, 125, 61);
		ActionCost buildStarPortCost = ActionCost(150, 100, 36);
		ActionCost buildTankCost = ActionCost(150, 125, 32);
		ActionCost buildVikingCost = ActionCost(150, 75, 30);

		void advanceConstructions();
		void advanceResources();
		void advanceOccupiedWorkers();
		void advanceEnemyAction();
		void advanceTime();

		bool hasEnoughMinerals(const int cost) const { return _minerals >= cost; };
		bool hasEnoughVespene(const int cost) const { return _vespene >= cost; }
		bool hasUnoccupiedWorker() const { return _workerPopulation - _occupiedWorkerTimers.size() > 0; }

		void occupyWorker(int time) {
			_occupiedWorkerTimers.emplace_back(time);
		};

		void addVespeneCollector();

		void addBase() {
			_populationLimit += 15;
			_incomingBases--;
			_populationLimit = _populationLimit >= MAX_POPULATION_LIMIT ? MAX_POPULATION_LIMIT : _populationLimit;
			_bases.emplace_back();
		}

		void addWorker() {
			_workerPopulation += 1;
			_incomingWorkers -= 1;
		}

		void addMarine() {
			_marinePopulation += 1;
			_incomingMarines -= 1;
		}

		void addTank() {
			_tankPopulation += 1;
			_incomingTanks -= 1;
		}

		void addViking() {
			_vikingPopulation += 1;
			_incomingVikings -= 1;
		}

		void addBarracks() {
			_barracksAmount += 1;
			_incomingBarracks = false;
		}

		void addFactory() {
			_factoryAmount += 1;
			_incomingFactory -= 1;
		}

		void addStarPort() {
			_starPortAmount += 1;
		}

		void addHouse() {
			_populationLimit += 8;
			_populationLimit = _populationLimit >= MAX_POPULATION_LIMIT ? MAX_POPULATION_LIMIT : _populationLimit;
			_hasHouse = true;
			_incomingHouse = false;
		}

		/*
		 *This method is not currently being used, but is kept for future purposes.
		 */
		void destroyPlayerBase() {
			_workerPopulation = _workerPopulation >= 10 ? _workerPopulation - 10 : 0;
			// _marinePopulation -= _marinePopulation >= 10 ? _marinePopulation - 10 : 0;
			// _barracksAmount = _barracksAmount <= 0 ? 0 : _barracksAmount - 1;

			if (!_bases.empty()) {
				_bases.pop_back();
				_populationLimit -= 15;
			}
		};

	public:
		int id = 0;
		[[nodiscard]] int getMinerals() const { return _minerals; }
		void setMinerals(int minerals) { _minerals = minerals; }
		[[nodiscard]] int getVespene() const { return _vespene; }

		[[nodiscard]] int getIncomingPopulation() const {
			return _incomingWorkers + _incomingMarines + _incomingTanks * TANK_SUPPLY + _incomingVikings *
			       VIKING_SUPPLY;
		}

		[[nodiscard]] int getIncomingWorkers() const { return _incomingWorkers; }
		[[nodiscard]] int getIncomingMarines() const { return _incomingMarines; }
		[[nodiscard]] int getIncomingTanks() const { return _incomingTanks; }
		[[nodiscard]] int getIncomingVikings() const { return _incomingVikings; }
		[[nodiscard]] int getPopulationLimit() const { return _populationLimit; }

		[[nodiscard]] int getPopulation() const {
			return _workerPopulation + _marinePopulation + _tankPopulation * TANK_SUPPLY + _vikingPopulation *
			       VIKING_SUPPLY;
		}

		[[nodiscard]] int getWorkerPopulation() const { return _workerPopulation; }
		[[nodiscard]] int getMarinePopulation() const { return _marinePopulation; }
		[[nodiscard]] int getTankPopulation() const { return _tankPopulation; }
		[[nodiscard]] int getVikingPopulation() const { return _vikingPopulation; }
		[[nodiscard]] int getOccupiedPopulation() const { return static_cast<int>(_occupiedWorkerTimers.size()); }
		[[nodiscard]] int getEnemyCombatUnits() const { return _enemyCombatUnits; }
		[[nodiscard]] Enemy getEnemy() const { return _enemy; }
		[[nodiscard]] std::list<Construction> getConstructions() const { return _constructions; }
		[[nodiscard]] std::vector<Base> getBases() const { return _bases; }
		[[nodiscard]] int getBarracksAmount() const { return _barracksAmount; }
		[[nodiscard]] int getFactoryAmount() const { return _factoryAmount; }
		[[nodiscard]] int getStarPortAmount() const { return _starPortAmount; }
		[[nodiscard]] bool getHasHouse() const { return _hasHouse; }
		[[nodiscard]] bool getIncomingHouse() const { return _incomingHouse; }
		[[nodiscard]] bool getIncomingBarracks() const { return _incomingBarracks; }
		[[nodiscard]] int getIncomingFactory() const { return _incomingFactory; }
		[[nodiscard]] int getIncomingBases() const { return _incomingBases; }
		[[nodiscard]] int getMaxBases() const { return MAX_BASES; }

		ActionCost getBuildWorkerCost() const { return buildWorkerCost; }
		ActionCost getBuildBaseCost() const { return buildBaseCost; }
		ActionCost getBuildHouseCost() const { return buildHouseCost; }
		ActionCost getBuildVespeneCollectorCost() const { return buildVespeneCollectorCost; }
		ActionCost getBuildBarracksCost() const { return buildBarracksCost; }
		ActionCost getBuildMarineCost() const { return buildMarineCost; }

		bool hasUnoccupiedGeyser() const;
		bool canAffordConstruction(const ActionCost &actionCost) const;
		bool populationLimitReached() const;
		bool withinPopulationLimit(int populationIncrease) const;
		bool hasFreeBase() const;
		bool hasFreeBarracks() const;
		bool hasFreeFactory() const;
		bool hasFreeStarPort() const;

		int mineralGainedPerTimestep() const;
		int vespeneGainedPerTimestep() const;

		int getMineralWorkers() const;
		int getVespeneWorkers() const;
		int getScoutWorkers() const;

		void buildWorker();
		void buildHouse();

		void buildBase();
		void buildVespeneCollector();
		void buildBarracks();
		void buildFactory();
		void buildStarPort();
		void buildMarine();
		void buildTank();
		void buildViking();
		void addEnemyUnit() { _enemyCombatUnits += 1; }
		void addEnemyGroundPower() { _enemy.groundPower += std::floor(_enemy.groundProduction); }
		void addEnemyAirPower() { _enemy.airPower += std::floor(_enemy.airProduction); }
		void addEnemyGroundProduction() { _enemy.groundProduction += 0.1; }
		void addEnemyAirProduction() { _enemy.airProduction += 0.1; }

		void attackPlayer() {
			auto temp = getValue();
			if (temp < .4) {
				_wasAttacked = true;
			}
		}

		void wait();
		void wait(int amount);

		int getVespeneCollectorsAmount() const;
		int getVespeneGeysersAmount();

		void performAction(const Action action) {
			switch (action) {
				case Action::buildWorker:
					buildWorker();
					break;
				case Action::buildHouse:
					buildHouse();
					break;
				case Action::buildBase:
					buildBase();
					break;
				case Action::buildVespeneCollector:
					buildVespeneCollector();
					break;
				case Action::buildBarracks:
					buildBarracks();
					break;
				case Action::buildMarine:
					buildMarine();
					break;
				case Action::buildFactory:
					buildFactory();
					break;
				case Action::buildTank:
					buildTank();
					break;
				case Action::buildViking:
					buildViking();
					break;
				case Action::buildStarPort:
					buildStarPort();
					break;
				case Action::none:
				case Action::attackPlayer:
				case Action::addEnemyUnit:
					break;
				default:
					throw std::runtime_error("Could not perform action: " + actionToString(action));
			}
		}

		std::vector<Action> getLegalActions() const;


		double getValueMarines() const {
			return softmax(std::vector{
				               static_cast<double>(_marinePopulation) / 4, static_cast<double>(_enemyCombatUnits) / 4
			               }, 0);
		}

		double getValueArmyPowerScaled() const {
			//this assumes the vikings can attack both air and ground at the same time
			const auto airPower = calculateAirPower();
			const auto groundPower = calculateGroundPower();
			const auto groundSoftMax = softmax(std::vector{
				                                   static_cast<double>(groundPower) / 4,
				                                   static_cast<double>(_enemy.groundPower) / 4
			                                   }, 0);
			const auto airSoftMax = softmax(std::vector{
				                                static_cast<double>(airPower) / 4,
				                                static_cast<double>(_enemy.airPower) / 4
			                                }, 0);
			const auto average = (airSoftMax + groundSoftMax) / 2;
			const auto difference = std::abs(airSoftMax - groundSoftMax);
			return average * (1 - difference);
		}

		double getValueArmyPowerAverage() const {
			//this assumes the vikings can attack both air and ground at the same time
			const auto airPower = calculateAirPower();
			const auto groundPower = calculateGroundPower();
			const auto groundSoftMax = softmax(std::vector{
				                                   static_cast<double>(groundPower) / 4,
				                                   static_cast<double>(_enemy.groundPower) / 4
			                                   }, 0);
			const auto airSoftMax = softmax(std::vector{
				                                static_cast<double>(airPower) / 4,
				                                static_cast<double>(_enemy.airPower) / 4
			                                }, 0);
			const auto average = (airSoftMax + groundSoftMax) / 2;
			return average;
		}

		double getValueMinArmyPower() const {
			const auto airPower = calculateAirPower();
			const auto groundPower = calculateGroundPower();
			const auto groundSoftMax = softmax(std::vector{
				                                   static_cast<double>(groundPower) / 4,
				                                   static_cast<double>(_enemy.groundPower) / 4
			                                   }, 0);
			const auto airSoftMax = softmax(std::vector{
				                                static_cast<double>(airPower) / 4,
				                                static_cast<double>(_enemy.airPower) / 4
			                                }, 0);
			return std::min(groundSoftMax, airSoftMax);
		}

		int calculateGroundPower() const {
			return _marinePopulation * unitGroundPower.at(UnitType::Marine) +
			       _tankPopulation * unitGroundPower.at(UnitType::Tank) +
			       _vikingPopulation * unitGroundPower.at(UnitType::Viking);
		}

		int calculateAirPower() const {
			return _marinePopulation * unitAirPower.at(UnitType::Marine) +
			       _tankPopulation * unitAirPower.at(UnitType::Tank) +
			       _vikingPopulation * unitAirPower.at(UnitType::Viking);
		}

		double calculateFloatingResourcesValue() const {
			//1 if 0 and 0 if 1000
			const auto mineralValue = 1 - std::min(_minerals / 1000.0, 1.0);
			//1 if 0 and 0 if 500
			const auto gasValue = 1 - std::min(_vespene / 500.0, 1.0);
			return (gasValue + mineralValue) / 2.0;
		}

		double calculateResourceGatheredValue() const {
			//53+27
			const auto resourcesGathered = mineralGainedPerTimestep() + vespeneGainedPerTimestep() * 1.5;
			return std::min(resourcesGathered / 80, 1.0);
		}

		double getAllValueTypes() const {
			const auto armyValue = getValueMinArmyPower();
			const auto floatingValue = calculateFloatingResourcesValue();
			const auto resourceValue = calculateResourceGatheredValue();
			return armyValue * 0 + floatingValue * 0 + resourceValue * 1;
		}

		double getValue() const {
			return getValueMinArmyPower();
		}

		std::tuple<double, double, double> getWinProbabilities();

		double getCombatSuccessProbability() const;
		double getEndProbability() const;

		static double softmax(std::vector<double> vector, const int index) {
			double sum = 0;
			for (const auto &value: vector) {
				sum += std::exp(value);
			}
			return std::exp(vector.at(index)) / sum;
		}

		std::vector<int> &getOccupiedWorkerTimers() { return _occupiedWorkerTimers; }


		bool endTimeReached() const {
			return _currentTime >= _endTime;
		}

		bool GameOver() const {
			return endTimeReached() || getEndProbability() > 0.90;
		}

		int getCurrentTime() const { return _currentTime; }
		void resetCurrentTime() { _currentTime = 0; }

		Action generateEnemyAction() {
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

		void setEndProbabilityFunction(const int endProbabilityFunction) {
			END_PROBABILITY_FUNCTION = endProbabilityFunction;
		}

		void setArmyValueFunction(const ArmyValueFunction army_value_function) {
			_armyValueFunction = army_value_function;
		};
		static std::shared_ptr<State> DeepCopy(const State &state, bool onRollout = false);

		static std::shared_ptr<State> StateBuilder(const int minerals,
		                                           const int vespene,
		                                           const int workerPopulation,
		                                           const int marinePopulation,
		                                           const int tankPopulation,
		                                           const int vikingPopulation,
		                                           const int incomingWorkers,
		                                           const int incomingMarines,
		                                           const int incomingTanks,
		                                           const int incomingVikings,
		                                           const int populationLimit,
		                                           std::vector<Base> &bases,
		                                           const int barracksAmount,
		                                           const int factoryAmount,
		                                           const int starPortAmount,
		                                           std::list<Construction> &constructions,
		                                           std::vector<int> &occupiedWorkerTimers,
		                                           const int currentTime,
		                                           const int endTime,
		                                           const int enemyCombatUnits,
		                                           const bool hasHouse,
		                                           const bool incomingHouse,
		                                           const bool incomingBarracks,
		                                           const int incomingFactory,
		                                           const int incomingBases,
		                                           const int maxBases,
		                                           const Enemy &enemy
		) {
			const unsigned int seed = std::random_device{}();
			StateBuilderParams params = {
				minerals,
				vespene,
				workerPopulation,
				marinePopulation,
				tankPopulation,
				vikingPopulation,
				incomingWorkers,
				incomingMarines,
				incomingTanks,
				incomingVikings,
				populationLimit,
				bases,
				barracksAmount,
				factoryAmount,
				starPortAmount,
				constructions,
				occupiedWorkerTimers,
				currentTime,
				endTime,
				enemyCombatUnits,
				hasHouse,
				incomingHouse,
				incomingBarracks,
				incomingFactory,
				incomingBases,
				maxBases,
				enemy
			};
			return InternalStateBuilder(params, -1, ArmyValueFunction::None, seed);
		}

		/*
		 * An internal version of the StateBuilder, which has enemyActions and combatBiases as pointers, as well as a seed.
		 * These cannot be set via the python script.
		 */
		static std::shared_ptr<State> InternalStateBuilder(StateBuilderParams params,
		                                                   const unsigned int endProbabilityFunction,
		                                                   const ArmyValueFunction armyValueFunction,
		                                                   unsigned int seed) {
			auto state = std::make_shared<State>(params, endProbabilityFunction, armyValueFunction, seed);

			for (auto &construction: params.constructions) {
				construction.setState(state);
				state->_constructions.emplace_back(construction);
			}

			return state;
		};

		State(const StateBuilderParams &params, const int endProbabilityFunction,
		      const ArmyValueFunction armyValueFunction,
		      const unsigned int seed): _armyValueFunction(armyValueFunction),
		                                END_PROBABILITY_FUNCTION(endProbabilityFunction),
		                                _minerals(params.minerals),
		                                _vespene(params.vespene),
		                                _workerPopulation(params.workerPopulation),
		                                _marinePopulation(params.marinePopulation),
		                                _tankPopulation(params.tankPopulation),
		                                _vikingPopulation(params.vikingPopulation),
		                                _incomingWorkers(params.incomingWorkers),
		                                _incomingMarines(params.incomingMarines),
		                                _incomingTanks(params.incomingTanks),
		                                _incomingVikings(params.incomingVikings),
		                                MAX_BASES(params.maxBases),
		                                _populationLimit(params.populationLimit),
		                                _barracksAmount(params.barracksAmount),
		                                _factoryAmount(params.factoryAmount),
		                                _starPortAmount(params.starPortAmount),
		                                _bases(std::move(params.bases)),
		                                _constructions(std::list<Construction>()),
		                                _occupiedWorkerTimers(
			                                std::move(params.occupiedWorkerTimers)),
		                                _enemyCombatUnits(params.enemyCombatUnits),
		                                _endTime(params.endTime),
		                                _currentTime(params.currentTime),
		                                _hasHouse(params.hasHouse),
		                                _incomingHouse(params.incomingHouse),
		                                _incomingBarracks(params.incomingBarracks),
		                                _incomingFactory(params.incomingFactory),
		                                _incomingBases(params.incomingBases) {
			_rng = std::mt19937(seed);
		};

		State(const State &state) : enable_shared_from_this(state),
		                            _armyValueFunction(state._armyValueFunction),
		                            END_PROBABILITY_FUNCTION(state.END_PROBABILITY_FUNCTION),
		                            MAX_BASES(state.MAX_BASES),
		                            _endTime(state._endTime),
		                            _currentTime(state._currentTime) {
			_minerals = state._minerals;
			_vespene = state._vespene;
			_workerPopulation = state._workerPopulation;
			_marinePopulation = state._marinePopulation;
			_tankPopulation = state._tankPopulation;
			_vikingPopulation = state._vikingPopulation;
			_incomingWorkers = state._incomingWorkers;
			_incomingMarines = state._incomingMarines;
			_incomingTanks = state._incomingTanks;
			_incomingVikings = state._incomingVikings;
			_populationLimit = state._populationLimit;
			_incomingVespeneCollectors = state._incomingVespeneCollectors;
			_barracksAmount = state._barracksAmount;
			_factoryAmount = state._factoryAmount;
			_starPortAmount = state._starPortAmount;

			buildWorkerCost = state.buildWorkerCost;
			buildBaseCost = state.buildBaseCost;
			buildHouseCost = state.buildHouseCost;
			buildVespeneCollectorCost = state.buildVespeneCollectorCost;
			buildMarineCost = state.buildMarineCost;
			buildBarracksCost = state.buildBarracksCost;
			buildFactoryCost = state.buildFactoryCost;
			buildStarPortCost = state.buildStarPortCost;
			buildTankCost = state.buildTankCost;
			buildVikingCost = state.buildVikingCost;

			_bases = std::vector<Base>();
			_constructions = std::list<Construction>();
			_occupiedWorkerTimers = state._occupiedWorkerTimers;

			_enemyCombatUnits = state._enemyCombatUnits;

			_rng = state._rng;

			_hasHouse = state._hasHouse;
			_incomingHouse = state._incomingHouse;
			_incomingBarracks = state._incomingBarracks;
			_incomingFactory = state._incomingFactory;
			_incomingBases = state._incomingBases;
		};

		explicit State(const int endTime, const int endProbabilityFunction, const ArmyValueFunction armyValueFunction,
		               const unsigned int seed): _armyValueFunction(armyValueFunction),
		                                         END_PROBABILITY_FUNCTION(endProbabilityFunction),
		                                         _endTime(endTime) {
			_rng = std::mt19937(seed);
		}

		State(): _armyValueFunction(ArmyValueFunction::MinPower), END_PROBABILITY_FUNCTION(2),
		         _rng(std::mt19937(std::random_device{}())), _endTime(1000) {
		}

		std::string toString() const {
			std::ostringstream str;
			str << "State: " << id << "{\n"
					<< "    Minerals: " << _minerals << "\n"
					<< "    Vespene: " << _vespene << "\n"
					<< "    IncomingVespeneCollectors: " << _incomingVespeneCollectors << "\n"
					<< "    Constructions: " << _constructions.size() << "\n"
					<< "    Occupied workers: " << _occupiedWorkerTimers.size() << "\n"
					<< "    Population: " << getPopulation() << "\n"
					<< "    Marine population: " << _marinePopulation << "\n"
					<< "    PopulationLimit: " << _populationLimit << "\n"
					<< "    MaxPopulationLimit: " << MAX_POPULATION_LIMIT << "\n"
					<< "    MaxBases: " << MAX_BASES << "\n"
					<< "    IncomingPopulation: " << getIncomingPopulation() << "\n"
					<< "    IncomingMarines: " << getIncomingMarines() << "\n"
					<< "    Number of bases: " << _bases.size() << "\n"
					<< "    Number of barracks: " << _barracksAmount << "\n"
					<< "    Number of constructions: " << _constructions.size() << "\n"
					<< "    Enemy combat units: " << _enemyCombatUnits << "\n"
					<< "    current_time: " << _currentTime << "\n"
					<< "    EndTime: " << _endTime << "\n"
					<< "    HasHouse: " << _hasHouse << "\n"
					<< "}\n";

			return str.str();
		}

		friend Construction;
	};

	inline std::ostream &operator<<(std::ostream &os, const State &state) {
		os << state.toString();
		return os;
	}
}
