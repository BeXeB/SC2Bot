//
// Created by marco on 05/12/2024.
//

#ifndef ROLLOUTHEURISTICENUM_H
#define ROLLOUTHEURISTICENUM_H
enum class RolloutHeuristic{
  Random,
  WeightedChoice,
};
inline std::ostream &operator<<(std::ostream &os, const RolloutHeuristic &heuristic) {
  switch (heuristic) {
    case RolloutHeuristic::Random:
      os << "Random";
    break;
    case RolloutHeuristic::WeightedChoice:
      os << "WeightedChoice";
    break;
    default:
      os << "Unknown";
    break;
  }
  return os;
}
#endif //ROLLOUTHEURISTICENUM_H
