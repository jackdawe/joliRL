#include "discreteaction.h"

DiscreteAction::DiscreteAction()
{

}

DiscreteAction::DiscreteAction(int size): size(size)
{

}

float DiscreteAction::pick()
{
    default_random_engine generator(random_device{}());
    uniform_int_distribution<int> dist(0,size-1);
    return dist(generator);
}

int DiscreteAction::getSize() const
{
    return size;
}
