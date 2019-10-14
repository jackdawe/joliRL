#include "controllerss.h"

ControllerSS::ControllerSS()
{

}

ControllerSS::ControllerSS(string mapTag)
{
    MapSS map;
    map.load(mapTag);
    planets = map.getPlanets();
    waypoints = map.getWaypoints();
    ship = map.getShip();
    vector<DiscreteAction> dactions = {DiscreteAction(waypoints.size()+1)};
    vector<ContinuousAction> cactions {ContinuousAction(0,SHIP_MAX_THRUST), ContinuousAction(0,2*M_PI)};
    actions = ActionSpace(dactions,cactions);
    takenAction = vector<float>(3,0);
}

ControllerSS::ControllerSS(string mapTag, Ship s)
{

}

float ControllerSS::transition()
{
    float thrustPow = takenAction[0];
    float thrustOri = takenAction[1];
    int signal = (int)takenAction[2];

    if (!isTerminal(currentState))
    {
        ship.setThrust(Vect2d(cos(thrustOri),sin(thrustOri)).dilate(thrustPow));
        ship.setSignalColor(signal);
    }
    Vect2d gravForce(0,0);
    for (unsigned int i=0;i<planets.size();i++)
    {
        Planet p = planets[i];
        Vect2d vectPS = ship.getP().sum(p.getCentre());
        gravForce.sum(vectPS.dilate(GRAVITY*SHIP_MASS*p.getMass()/pow(vectPS.norm(),3)));
        ship.setA(Vect2d(gravForce.getX()-DAMPING*ship.getV().getX()-ship.getThrust().getX(),gravForce.getY()-DAMPING*ship.getV().getY()-ship.getThrust().getY()).dilate(1./SHIP_MASS));
        ship.setP(ship.getP().sum(ship.getV().dilate(STEP_SIZE)));
        ship.setV(ship.getV().sum(ship.getA().dilate(STEP_SIZE)));
    }
    updateStateVector();

    for (unsigned int i=0;i<planets.size();i++)
    {
        if (ship.getP().distance(planets[i].getCentre()) < ship.getWidth()+planets[i].getRadius())
        {
            return CRASH_REWARD;
        }
    }
    if (ship.getSignalColor() != actions.getDiscreteActions()[0].getSize())
    {
        for (unsigned int i=0;i<waypoints.size();i++)
        {
            if (ship.getP().distance(waypoints[i].getCentre()) < waypoints[i].getRadius())
            {
                if (ship.getSignalColor() == i)
                {
                    return RIGHT_SIGNAL_ON_WAYPOINT_REWARD;
                }
                else
                {
                    return WRONG_SIGNAL_ON_WAYPOINT_REWARD;
                }
            }
            return SIGNAL_OFF_WAYPOINT_REWARD;
        }
    }
}

bool ControllerSS::isTerminal(State s)
{

}

void ControllerSS::updateStateVector()
{

}

int ControllerSS::stateId(State s)
{

}

void ControllerSS::reset()
{

}

vector<int> ControllerSS::accessibleStates(State s)
{

}

int ControllerSS::spaceStateSize()
{

}
