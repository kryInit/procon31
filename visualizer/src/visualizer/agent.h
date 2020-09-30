#ifndef agent_h
#define agent_h

#include "action.h"

class Agent {
    int agentID;
    Vec2 coord;
    Action action;
    
public:
    Agent() { agentID = -1; coord = Vec2(-1, -1); action = Action(); }
    Agent(int _agentID, Vec2 _coord) { agentID = _agentID; coord = _coord; action = Action(); }
    
    void setCoord(Vec2 _coord);
    void setAction(Action _action);

    int getAgentID();
    Vec2 getCoord();
    Action getAction();
    
    bool hasActiveAction();
    bool isSettableAgent();
};

inline void Agent::setCoord(Vec2 _coord) { coord = _coord; }
inline void Agent::setAction(Action _action) { action = _action; }

inline int Agent::getAgentID() { return agentID; }
inline Vec2 Agent::getCoord() {return coord; }
inline Action Agent::getAction() { return action; }

inline bool Agent::hasActiveAction() { return action.isActiveAction(); }
inline bool Agent::isSettableAgent() { return coord.x == -1 && coord.y == -1 && agentID >= 1; }

#endif
