#include "Sc2State.h"
#include <iostream>
// #include <ostream>

int main()
{
    std::cout << "Hello World!\n";
    auto state = sc2::Sc2State(1, "slim shady");
    state.printName();
}
void sc2::Sc2State::printName() {
    std::cout << name << std::endl;
}
