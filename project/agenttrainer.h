#ifndef AGENTTRAINER_H
#define AGENTTRAINER_H
#include "agent.h"
#include <iostream>

template <class A>
class AgentTrainer
{
public:
    AgentTrainer();
    void train(A* agent, int numberOfEpisodes,int trainMode = 0,int savingSequenceMode = 0);
    void saveEpisode(vector<vector<double>> actionSequence, int seqId);
};

#endif // AGENTTRAINER_H
