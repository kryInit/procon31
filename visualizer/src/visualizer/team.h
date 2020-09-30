#ifndef team_h
#define team_h

#include <vector>
#include "agent.h"
#include "constant.h"

class Team {
    int teamID;
    int agentNum;
    int wallPoint, areaPoint;
    Agent agents[MAX_AGENT];
    Color agentColor, wallTileColor, areaTileColor;
    
public:
    Team() { teamID = 0; agentNum = 0; }
    Team(int _teamID, int _agentNum, Color _agentColor, Color _wallTileColor, Color _areaTileColor) {
        wallPoint = areaPoint = 0;
        teamID = _teamID; agentNum = _agentNum;
        agentColor = _agentColor; wallTileColor = _wallTileColor; areaTileColor = _areaTileColor;
        for(int i=0; i<MAX_AGENT; ++i) agents[i] = Agent();
    }
    
    void setAgent(int idx, Agent agent);
    void setAgentsAction(int idx, Action action);
    void setWallPoint(int _wallPoint);
    void setAreaPoint(int _areaPoint);
    
    int getTeamID();
    int getAgentNum();
    int getWallPoint();
    int getAreaPoint();
    Agent getAgent(int idx);

    int getAgentsIdxFromCoord(Vec2 coord);
    std::vector<Agent> getAgentsWithActiveAction();
    
    bool agentExists(Vec2 coord);
    int countSettableAgent();
    
    Color getAgentColor();
    Color getWallTileColor();
    Color getAreaTileColor();
};

inline void Team::setAgent(int idx, Agent agent) { agents[idx] = agent; }
inline void Team::setAgentsAction(int idx, Action action) { agents[idx].setAction(action); }
inline void Team::setWallPoint(int _wallPoint) { wallPoint = _wallPoint; }
inline void Team::setAreaPoint(int _areaPoint) { areaPoint = _areaPoint; }

inline int Team::getTeamID() { return teamID; }
inline int Team::getAgentNum() { return agentNum; }
inline int Team::getWallPoint() { return wallPoint; }
inline int Team::getAreaPoint() { return areaPoint; }
inline Agent Team::getAgent(int idx) { return agents[idx]; }

inline int Team::getAgentsIdxFromCoord(Vec2 coord) {
    for(int i=0; i<agentNum; ++i) if (agents[i].getCoord() == coord) return i;
    return -1;
}
inline std::vector<Agent> Team::getAgentsWithActiveAction() {
    std::vector<Agent> v;
    for(int i=0; i<agentNum; ++i) if (agents[i].hasActiveAction()) v.push_back(agents[i]);
    return v;
}

inline bool Team::agentExists(Vec2 coord) {
    return 0 <= getAgentsIdxFromCoord(coord);
}
inline int Team::countSettableAgent() {
    int cnt=0;
    for(int i=0; i<agentNum; ++i) if (agents[i].isSettableAgent()) cnt++;
    return cnt;
}

inline Color Team::getAgentColor() { return agentColor; }
inline Color Team::getWallTileColor() { return wallTileColor; }
inline Color Team::getAreaTileColor() { return areaTileColor; }

#endif
