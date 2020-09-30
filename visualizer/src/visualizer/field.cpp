#include <Siv3D.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "constant.h"
#include "field.h"
#include "team.h"

using namespace std;

extern bool CAN_USE_BLUE_FORCED_ACTIONS;
extern bool CAN_USE_ORANGE_FORCED_ACTIONS;
extern bool CAN_SHOW_ACTION[TEAM_NUM];
extern bool CAN_USE_PS4_CONTROLLER;

Field::Field() { height = width = turn = 0; }

Field::Field(int matchID, int myTeamID) {
    {
        ifstream ifs("../data/" + to_string(matchID) + "/fieldInfo");
        int _;
        ifs >> height >> width >> turn >> _;
        for(int i=0; i<height; ++i) for(int j=0; j<width; ++j) ifs >> walls[i][j];
        for(int i=0; i<height; ++i) for(int j=0; j<width; ++j) ifs >> areas[i][j];
        for(int i=0; i<height; ++i) for(int j=0; j<width; ++j) ifs >> points[i][j];

        for(int i=0; i<TEAM_NUM; ++i) {
            int teamID, agentNum, areaPoint, wallPoint;
            ifs >> teamID >> agentNum >> areaPoint >> wallPoint;

            int idx = (teamID == myTeamID ? 0 : 1);
            if (idx == 0) teams[0] = Team(teamID, agentNum, Color(153,204,255), Color(76,108,179), Color(60,230,255));
            else teams[1] = Team(teamID, agentNum, Color(245,229,107), Color(243,152,0), Color(255,255,130));
            

            teams[idx].setAreaPoint(areaPoint);
            teams[idx].setWallPoint(wallPoint);
            for(int j=0; j<agentNum; ++j) {
                int x,y,agentID;
                ifs >> agentID >> x >> y;
                teams[idx].setAgent(j, Agent(agentID, Vec2(x,y)));
            }
        }
    }
    {
        ifstream ifs("../data/" + to_string(matchID) + "/forcedAction");
        int n;
        ifs >> n;
        for(int k=0; k<n; ++k) {
            int agentID, x, y;
            string type;
            ifs >> agentID >> x >> y >> type;
            for(int i=0; i<TEAM_NUM; ++i) {
                int agentNum = teams[i].getAgentNum();
                for(int j=0; j<agentNum; ++j) {
                    if (teams[i].getAgent(j).getAgentID() == agentID) {
                        teams[i].setAgentsAction(j, Action(type, Vec2(x,y)));
                    }
                }
            }
        }
    }
}
    
void Field::updateFieldInfo(int matchID) {
    ifstream ifs("../data/" + to_string(matchID) + "/fieldInfo");
    int _, nowTurn = turn;
    ifs >> _ >> _ >> turn >> _;
    if (nowTurn == turn) return;
    for(int i=0; i<height; ++i) for(int j=0; j<width; ++j) ifs >> walls[i][j];
    for(int i=0; i<height; ++i) for(int j=0; j<width; ++j) ifs >> areas[i][j];
    for(int i=0; i<height; ++i) for(int j=0; j<width; ++j) ifs >> points[i][j];

    for(int i=0; i<TEAM_NUM; ++i) {
        int idx = i;
        int teamID, agentNum, areaPoint, wallPoint;
        ifs >> teamID >> agentNum >> areaPoint >> wallPoint;
        if (teamID != teams[i].getTeamID()) idx = (i^1);
        teams[idx].setAreaPoint(areaPoint);
        teams[idx].setWallPoint(wallPoint);
        for(int j=0; j<agentNum; ++j) {
            int x,y,agentID;
            ifs >> agentID >> x >> y;
            teams[idx].setAgent(j, Agent(agentID, Vec2(x,y)));
        }
    }
}

void Field::setAction2CloseAgent(Vec2 coord, Vec2 dcoord, string type) {
    int nowMinDist = MAX_SIDE*2;
    int teamIdx = -1, agentIdx = -1;

    for(int i=0; i<TEAM_NUM; ++i) {
        if (i==0 && !CAN_USE_BLUE_FORCED_ACTIONS) continue;
        if (i==1 && !CAN_USE_ORANGE_FORCED_ACTIONS) continue;
        for(int j=0; j<teams[i].getAgentNum(); ++j) {
            Vec2 tmpCoord = teams[i].getAgent(j).getCoord();
            int d = abs(tmpCoord.x-coord.x) + abs(tmpCoord.y-coord.y);
            if (isSafeIdx(tmpCoord) && isSafeIdx(tmpCoord+dcoord) && d < nowMinDist) {
                nowMinDist = d;
                teamIdx = i;
                agentIdx = j;
            }
        }
    }
    if (teamIdx >= 0 && agentIdx >= 0) {
        Vec2 targetCoord = teams[teamIdx].getAgent(agentIdx).getCoord() + dcoord;
        teams[teamIdx].setAgentsAction(agentIdx, Action(type, targetCoord));
    }
}

void Field::setActionWithLine(Vec2 s, Action action) {
    for(int i=0; i<TEAM_NUM; ++i) {
        if (i==0 && !CAN_USE_BLUE_FORCED_ACTIONS) continue;
        if (i==1 && !CAN_USE_ORANGE_FORCED_ACTIONS) continue;
        for(int j=0; j<teams[i].getAgentNum(); ++j) {
            if (teams[i].getAgent(i).getCoord() == s) {
                teams[i].setAgentsAction(i, action);
            }
        }
    }
}

void Field::assignSETAction(int teamIdx, Vec2 coord, map<int, Action> algosAction) {
    if (teamIdx==0 && !CAN_USE_BLUE_FORCED_ACTIONS) return;
    if (teamIdx==1 && !CAN_USE_ORANGE_FORCED_ACTIONS) return;
    Action action(SET, coord);
    Team team = teams[teamIdx];
    for(int i=0; i<team.getAgentNum(); ++i) {
        Agent agent = team.getAgent(i);
        if (agent.isSettableAgent() && ((!agent.hasActiveAction() && !algosAction.count(agent.getAgentID())) || agent.getAction().getType() == STAY)) {
            teams[teamIdx].setAgentsAction(i, action);
            return;
        }
    }
}

void Field::cancelAction(int teamIdx, Vec2 coord, std::map<int, Action> algosAction) {
    if (teamIdx==0 && !CAN_USE_BLUE_FORCED_ACTIONS) return;
    if (teamIdx==1 && !CAN_USE_ORANGE_FORCED_ACTIONS) return;
    Team team = teams[teamIdx];
    for(int i=0; i<team.getAgentNum(); ++i) {
        Agent agent = team.getAgent(i);
        if (agent.getAction().getTargetCoord() == coord) {
            teams[teamIdx].setAgentsAction(i, Action());
        }
        if (algosAction[agent.getAgentID()].getTargetCoord() == coord) {
            teams[teamIdx].setAgentsAction(i, Action(STAY, agent.getCoord()));
        }
    }
}

vector<Agent> Field::getAllTeamsAgentsWithActiveAction() {
    vector<Agent> result;
    for(int i=0; i<TEAM_NUM; ++i) {
        vector<Agent> tmp = teams[i].getAgentsWithActiveAction();
        copy(tmp.begin(), tmp.end(), back_inserter(result));
    }
    return result;
}
