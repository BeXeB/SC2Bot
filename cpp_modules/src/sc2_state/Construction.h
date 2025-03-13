//
// Created by marco on 31/10/2024.
//
#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H
// #include <format>
#include <memory>
#include <utility>
#include "ActionEnum.h"


namespace Sc2 {
    class State;

    typedef void (State::*ConstructionFunction)();

    class Construction {
        int _timeLeft = 0;
        bool _isFinished = false;
        std::weak_ptr<State> _state;
        Action _action = Action::none;
        ConstructionFunction _constructionFunction;

    public:
        void setState(std::weak_ptr<State> state) { _state = std::move(state); }

        Construction(const int constructionTime, std::weak_ptr<State> state,
                     const ConstructionFunction constructionFunction): _timeLeft(constructionTime),
                                                                       _state(std::move(state)),
                                                                       _constructionFunction(constructionFunction) {
        }

        Construction(const int constructionTime, const Action action);

        [[nodiscard]] int getTimeLeft() const { return _timeLeft; }
        [[nodiscard]] bool getIsFinished() const { return _isFinished; }
        [[nodiscard]] ConstructionFunction getConstructionFunction() const { return _constructionFunction; }

        void advanceTime(const int time) {
            _timeLeft -= time;
            if (_timeLeft <= 0) {
                const auto state = _state.lock();

                if (state != nullptr) {
                    (state.get()->*_constructionFunction)();
                    _isFinished = true;
                }
            }
        }

        [[nodiscard]] std::string toString() const {
            // std::string str = "Construction: {\n";
            // str += std::format("    timeLeft: {} \n", _timeLeft);
            // str += std::format("    action: {} \n", actionToString(_action));
            // str += "}";

            return "str";
        }


        friend State;
    };
}

#endif //CONSTRUCTION_H
