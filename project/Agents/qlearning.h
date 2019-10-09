#ifndef QLEARNING_H
#define QLEARNING_H
#include "agent.h"

template <class C>
class QLearning: public Agent<C>
{
public:
    QLearning();
    QLearning(vector<Action> actionSpace, float epsilon, float gamma);
    void greedyPolicy();
    void updatePolicy();
    void savePolicy();
    void loadPolicy();
    void saveTrainingData();

private:
    float gamma;
    vector<double> qvalues;
};

#endif // QLEARNING_H