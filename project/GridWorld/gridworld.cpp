#include "gridworld.h"

GridWorld::GridWorld(){}

GridWorld::GridWorld(string pathToMap):
  randomStart(true), mapPoolSize(-1)
{
  map.load(pathToMap);
  init();
}

GridWorld::GridWorld(string pathToMap, float agentXInit, float agentYInit):
  randomStart(false),initX(agentXInit), initY(agentYInit), agentX(agentXInit),agentY(agentYInit),mapPoolSize(-1)
{
  map.load(pathToMap);
  init();
}

GridWorld::GridWorld(string mapPoolPath, int mapPoolSize):
  mapPoolPath(mapPoolPath), randomStart(true), mapPoolSize(mapPoolSize)
{
  map.load(mapPoolPath+"map0");
  init();
}

void GridWorld::init()
{
  this->tag = "../temp/gw_";
  vector<DiscreteAction> dactions = {DiscreteAction(4)};
  actions = ActionSpace(dactions, vector<ContinuousAction>());
  takenAction = vector<float>(1,0);
  size = map.getSize();
  for (int i=0;i<4+size*size;i++)
    {
      currentState.add(0);
    }
  for (int i=0;i<size;i++)
    {
      obstacles.push_back(vector<float>(map.getSize(),0));
    }
  reset();
}

float GridWorld::transition()
{
    int a = (int)takenAction[0];
    float r = 0;
    previousState.update(0,agentX), previousState.update(1,agentY);
    if (!isTerminal(currentState))
    {
        switch (a)
        {
        case 0:
            agentX--;
            break;
        case 1:
            agentY++;
            break;
        case 2:
            agentX++;
            break;
        case 3:
            agentY--;
            break;
        }
        currentState.update(0,agentX), currentState.update(1,agentY);
        actionSequence.push_back({a});
        stateSequence.push_back(currentState.getStateVector());
    }   
    if (obstacles[agentX][agentY] == 1)
    {
        r = LOSE_REWARD;
    }
    else if (agentX == goalX && agentY == goalY)
    {
        r = WIN_REWARD;
    }
    else
    {
        r = EMPTY_SQUARE_REWARD;
    }
    rewardHistory.back()+= r;
    return r;
}

bool GridWorld::isTerminal(State s)
{
    float ax = s.getStateVector()[0];
    float ay = s.getStateVector()[1];
    return obstacles[ax][ay] == 1 || (ax == goalX && ay == goalY);
}

void GridWorld::generateVectorStates()
{
    currentState.update(0,agentX),currentState.update(1,agentY),
            currentState.update(2,goalX), currentState.update(3,goalY);
    for (int i=0;i<size;i++)
    {
        for (int j=0;j<size;j++)
        {
            currentState.update(4+i*size+j,obstacles[i][j]);
        }
    }
    previousState = State(currentState);
    stateSequence.push_back(currentState.getStateVector());
}

cv::Mat GridWorld::toRGBMat(vector<float> stateVector)
{
    cv::Mat rgbState(size,size,CV_8UC3);
    for (int i=0;i<size;i++)
    {
        for (int j=0;j<size;j++)
        {
            for (int k=0;k<3;k++)
            {
                rgbState.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,0);
            }
        }
    }

    for (int i=0;i<size;i++)
    {
        for (int j=0;j<size;j++)
        {
            if (stateVector[i*size+j+4] == 1)
            {
                rgbState.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,255);
            }
        }
    }

    rgbState.at<cv::Vec3b>(stateVector[0],stateVector[1]) += cv::Vec3b(255,0,0);
    rgbState.at<cv::Vec3b>(stateVector[2],stateVector[3]) += cv::Vec3b(0,255,0);
    return rgbState;
}

torch::Tensor GridWorld::toRGBTensor(vector<float> stateVector)
{
  torch::Tensor rgbState = torch::zeros({3,size,size});
  for (int i=0;i<size;i++)
    {
      for (int j=0;j<size;j++)
	{
	  rgbState[2][i][j] = stateVector[i*size+j+4];
	}
    }
  rgbState[0][stateVector[0]][stateVector[1]] = 1;
  rgbState[1][stateVector[2]][stateVector[3]] = 1;
  return rgbState;
}

int GridWorld::stateId(State s)
{
    float ax = s.getStateVector()[0];
    float ay = s.getStateVector()[1];
    return ax*size+ay;
}

void GridWorld::reset()
{  
  rewardHistory.push_back(0);
  default_random_engine generator(std::random_device{}());
  uniform_int_distribution<int> dist(0,mapPoolSize-1);
  int mapId; 
  if (mapPoolSize != -1)
    {
      default_random_engine generator(std::random_device{}());
      uniform_int_distribution<int> dist(0,mapPoolSize-1);
      mapId = dist(generator);
      map.load(mapPoolPath+"map"+to_string(mapId));
      size = map.getSize();
      for (int i=0;i<size;i++)
	{
	  for (int j=0;j<size;j++)
	    {
	      switch(map.getMap()[i][j])
		{
		case 1:
		  obstacles[i][j]=1;
		  break;
		case 2:
		  goalX=i, goalY=j;
		  obstacles[i][j]=0;
		  break;
		default:
		  obstacles[i][j]=0;
		}
	    }
	}
    }
  if(randomStart)
    {
      default_random_engine generator(std::random_device{}());
      uniform_int_distribution<int> dist(1,size-1);
      agentX = dist(generator), agentY = dist(generator);
      while ((agentX == goalX && agentY == goalY) || obstacles[agentX][agentY] == 1)
	{
	  agentX = dist(generator), agentY = dist(generator);
	}
    }
  else
    {
      agentX = initX; agentY = initY;
    }
  generateVectorStates();
  actionSequence = vector<vector<float>>();
  stateSequence = {currentState.getStateVector()};
}

vector<int> GridWorld::accessibleStates(State s)
{
  int ax = s.getStateVector()[0];
  int ay = s.getStateVector()[1];
  vector<int> accessibleStates = {(ax-1)*size+ay,ax*size+ay+1,(ax+1)*size+ay,ax*size+ay-1};
  return accessibleStates;
}

int GridWorld::spaceStateSize()
{
  return size*size;
}

int GridWorld::getSize()
{
  return size;
}

vector<vector<float>> GridWorld::getObstacles()
{
  return obstacles;
}

float GridWorld::getAgentX()
{
  return agentX;
}

float GridWorld::getAgentY()
{
  return agentY;
}

float GridWorld::getGoalX()
{
  return goalX;
}

float GridWorld::getGoalY()
{
  return goalY;
}

void GridWorld::setAgentX(float x)
{
  agentX = x;
}

void GridWorld::setAgentY(float y)
{
  agentY = y;
}

void GridWorld::setGoalX(float x)
{
  goalX = x;
}

void GridWorld::setGoalY(float y)
{
  goalY = y;
}
