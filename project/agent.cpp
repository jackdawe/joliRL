#include "agent.h"

template <class W>
Agent<W>::Agent()
{
}

template <class W>
Agent<W>::Agent(W world):
  world(world)
{
    this->world.generateVectorStates();
}

template<class W>
void Agent<W>::saveRewardHistory()
{
    world.saveRewardHistory();
}

template<class W>
void Agent<W>::saveLastEpisode()
{
    world.saveLastEpisode();
}

template<class W>
void Agent<W>::loadEpisode(string nameTag)
{
    world.loadEpisode(nameTag);
}

template<class W>
int Agent<W>::daSize()
{
    return actions().getDiscreteActions().size();
}

template<class W>
int Agent<W>::caSize()
{
    return actions().getContinuousActions().size();
}

template<class W>
ActionSpace Agent<W>::actions()
{
    return world.getActions();
}

template<class W>
vector<DiscreteAction> Agent<W>::discreteActions()
{
    return actions().getDiscreteActions();
}

template<class W>
vector<ContinuousAction> Agent<W>::continuousActions()
{
    return actions().getContinuousActions();
}

template<class W>
State Agent<W>::previousState()
{
    return world.getPreviousState();
}

template<class W>
vector<float> Agent<W>::takenAction()
{
    return world.getTakenAction();
}

template<class W>
float Agent<W>::takenReward()
{
    return world.getTakenReward();
}

template<class W>
State Agent<W>::currentState()
{
    return world.getCurrentState();
}

template<class W>
vector<float> Agent<W>::rewardHistory()
{
  return world.getRewardHistory();
}

template<class W>
void Agent<W>::resetWorld()
{
  world.reset();
}

template<class W>
void Agent<W>::setWorld(const W &value)
{
    world = value;
}

template<class W>
W Agent<W>::getWorld() const
{
    return world;
}

template class Agent<GridWorld>;
template class Agent<SpaceWorld>;
