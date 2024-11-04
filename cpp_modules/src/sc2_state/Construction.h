//
// Created by marco on 31/10/2024.
//

#ifndef CONSTRUCTION_H
#define CONSTRUCTION_H
#include <memory>
#include <utility>


namespace Sc2 {
    class State;

    typedef void (State::*ConstructionFunction)();

    class Construction {
        int _timeLeft = 0;
        bool _isFinished = false;
        std::shared_ptr<State> _state;
        ConstructionFunction _constructionFunction;

        void setState(std::shared_ptr<State> state){_state = std::move(state);}

    public:
        Construction(const int constructionTime, std::shared_ptr<State> state, const ConstructionFunction constructionFunction):
            _timeLeft(constructionTime),
            _state(std::move(state)),
            _constructionFunction(constructionFunction) {}

        [[nodiscard]] int getTimeLeft() const {return _timeLeft;}
        [[nodiscard]] bool getIsFinished() const {return _isFinished;}
        [[nodiscard]] ConstructionFunction getConstructionFunction() const {return _constructionFunction;}

        void advanceTime(const int time) {
            _timeLeft -= time;
            if (_timeLeft <= 0) {

                (_state.get()->*_constructionFunction)();
                _isFinished = true;
            }
        }
        friend State;
    };
}

#endif //CONSTRUCTION_H
