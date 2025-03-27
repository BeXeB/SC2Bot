//
// Created by User on 27/03/2025.
//

#ifndef UNITPOWER_H
#define UNITPOWER_H
#include <map>

enum class UnitType {
	Marine,
	Tank,
	Viking,
};

static const std::map<UnitType, int> unitGroundPower = {
	{UnitType::Marine, 1},
	{UnitType::Tank, 3},
	{UnitType::Viking, 2}
};

static const std::map<UnitType, int> unitAirPower = {
	{UnitType::Marine, 1},
	{UnitType::Tank, 0},
	{UnitType::Viking, 2}
};

#endif //UNITPOWER_H
