#pragma once
#include <string>

namespace sc2 {
    struct Sc2State {
        int i = 1;
        std::string name = "slim shady";

        void printName();

        Sc2State(const int i, std::string name): i(i), name(std::move(name)) {}
    };
}
