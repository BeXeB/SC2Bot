//
// Created by marco on 31/10/2024.
//
#ifndef BASE_H
#define BASE_H
#include "Sc2Constants.h"

namespace Sc2 {
	struct Base {
		int id = 0;
		int mineralFields = 8;
		int vespeneGeysers = 2;
		int vespeneCollectors = 0;


		[[nodiscard]] int getUnoccupiedGeysers() const { return vespeneGeysers - vespeneCollectors; }
		[[nodiscard]] int getMineralWorkerLimit() const { return mineralFields * WORKER_PER_MINERAL_FIELD; }
		[[nodiscard]] int getVespeneWorkerLimit() const { return vespeneCollectors * WORKER_PER_GEYSER; }

		void addVespeneCollector() { if (vespeneCollectors < vespeneGeysers) vespeneCollectors++; }

		Base(const int id, const int mineralFields, const int vespeneGeysers, const int vespeneCollectors) : id(id),
			mineralFields(mineralFields),
			vespeneGeysers(vespeneGeysers),
			vespeneCollectors(vespeneCollectors) {
		}

		Base() = default;
	};
}

#endif //BASE_H
