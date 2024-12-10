#ifndef UCB1NORMAL2_H
#define UCB1NORMAL2_H

#include <vector>
#include <cmath>
#include <numeric>

class UCB1Normal2 {
private:
    // A vector of vectors (Buckets). Essentially, this is a vector of rewards for each arm
    std::vector<std::vector<double>> rewards;
    int totalTrials;

    double calculateMean(const std::vector<double>& samples);
    double calculateVariance(const std::vector<double>& samples, double mean);

public:
    explicit UCB1Normal2(int numArms);
    void addReward(int arm, double reward);
    int selectArm();
};

#endif // UCB1NORMAL2_H
