//
// Created by marco on 31/10/2024.
//
#ifndef BASE_H
#define BASE_H
#include <vector>

namespace Sc2 {
	class Collector;

	struct Base {
		int id = 0;
		// int amountOfWorkers = 0;
		int workerLimit = 15;
		std::vector<Collector> _vespeneCollectors{};
		// 		std::vector<int> position;

		Base(const int id, const int workerLimit) : id(id), workerLimit(workerLimit) {
		}

		Base() = default;
	};
}

#endif //BASE_H
