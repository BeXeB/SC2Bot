//
// Created by marco on 31/10/2024.
//
#ifndef BASE_H
#define BASE_H
#include <vector>

namespace Sc2 {
    struct Base{
        int id = 0;
        int amountOfWorkers = 0;
        int workerLimit = 0;
        std::vector<int> position;

        Base(const int id, const int amountOfWorkers, const int workerLimit ,std::vector<int> position) :
            id(id),
            amountOfWorkers(amountOfWorkers),
            workerLimit(workerLimit),
            position(std::move(position)) {}
    };
}

#endif //BASE_H
