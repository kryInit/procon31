#include <bits/stdc++.h>
#include "constant.hpp"
#include "classes.hpp"
using namespace std;

#define rep(i,n) for(int (i)=0; (i)<(n); ++(i))

extern int h, w, agentNum, turns, turn, operationMillis;
extern int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
extern Team teams[TEAM_NUM];

extern int matchID;
extern string dataPath;

void input() {
    {
        int _;
        ifstream ifs(dataPath + "gameInfo");
        ifs >> _ >> _ >> turns >> operationMillis;
    }
    {
        int _;
        ifstream ifs(dataPath + "fieldInfo");
        ifs >> h >> w >> turn >> _;
        rep(i,h) rep(j,w) ifs >> walls[i][j];
        rep(i,h) rep(j,w) ifs >> areas[i][j];
        rep(i,h) rep(j,w) ifs >> points[i][j];

        rep(i,TEAM_NUM) {
            ifs >> teams[i].teamID >> agentNum;
            ifs >> _ >> _;
            rep(j,agentNum) {
                ifs >> teams[i].agents[j].agentID >>  teams[i].agents[j].coord.x >> teams[i].agents[j].coord.y;
            }
        }
    }
}

void output() {
    ofstream ofs(dataPath + "action");
    int n=0;
    rep(i,TEAM_NUM) rep(j,agentNum) if (!teams[i].agents[j].action.type.empty()) ++n;
    ofs << n << endl;
    rep(i,TEAM_NUM) rep(j,agentNum) {
        Agent agent = teams[i].agents[j];
        Action action = agent.action;
        if (!agent.action.type.empty()) {
            ofs << agent.agentID << " " << action.targetCoord.x << " " << action.targetCoord.y << " " << action.type << endl;
        }
    }
}
