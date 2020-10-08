#include<bits/stdc++.h>
#include "constant.hpp"
#include "classes.hpp"
using namespace std;

#define rep(i,n) for(int i=0;i<(int)(n);++i)
#define rep1(i,n) for(int i=1;i<=(int)(n);++i)
#define rep11(i,n) for(int i=1;i<(int)(n);++i)
#define repo(i,o,n) for(int i=o;i<(int)(n);++i)
#define repm(i,n) for(int i=(int)(n)-1;i>=0;--i)


void input();
void output();

double calcAreaEV(Vec2 &topL, Vec2 &bottomR);
void enumeratePath(vector<pair<int,Path>> &paths, const int maxDepth, const int maxRadix, const int depthAtEnumeration, const int maxCntOfMoveToZero);


int h, w, agentNum, turns, turn;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];

int matchID;
string dataPath;

random_device rd;

int main(int argc, char *argv[]) {
    if (argc < 2) return 0;

    matchID = atoi(argv[1]);
    dataPath = (string)(((filesystem::path)(string(argv[0]))).parent_path()) + "/../../data/" + to_string(matchID) + "/";

    input();

    if (turn <= 2) {
        Vec2 topL, bottomR;
        double ev = calcAreaEV(topL, bottomR);
#ifdef DEBUG
        cout << ev << endl;
#endif
        if (ev > 0) {
            Vec2 tmp = topL;
            rep(i,agentNum) {
                if (turn != 1) teams[0].agents[i].action = Action((turn==0 ? SET : MOVE), tmp);
                rep(j,2) {
                    if (turn==1 && j==1) teams[0].agents[i].action = Action(MOVE, tmp);
                    if (tmp.y == topL.y) {
                        if (tmp.x == bottomR.x) tmp.y++;
                        else tmp.x++;
                    } else if (tmp.x == bottomR.x) {
                        if (tmp.y == bottomR.y) tmp.x--;
                        else tmp.y++;
                    } else if (tmp.y == bottomR.y) {
                        if (tmp.x == topL.x) tmp.y--;
                        else tmp.x--;
                    } else if (tmp.x == topL.x) {
                        if (tmp.y == topL.y) tmp.x++;
                        else tmp.y--;
                    }
                }
            }
        }
    } else {
        rep(i,agentNum) if (teams[0].agents[i].coord.x < 0 && teams[0].agents[i].coord.y < 0) teams[0].agents[i].action = Action(SET, Vec2(rd()%w, rd()%h));
        const int maxDepth = 7;
        const int maxRadix = 4;
        const int depthAtEnumeration = 2;
        const int maxCntOfMoveToZero = 1;

        vector<pair<int,Path>> v;

#ifdef DEBUG
        cout << "start!" << endl;
        clock_t start = clock();
#endif

        enumeratePath(v, maxDepth, maxRadix, depthAtEnumeration, maxCntOfMoveToZero);

#ifdef DEBUG
        cout << (double)(clock()-start) / CLOCKS_PER_SEC << "[s]" << endl;
#endif

        priority_queue<pair<int,Path>, vector<pair<int,Path>>, function<bool(pair<int,Path>,pair<int,Path>)>> pq(
                [](const pair<int,Path> &a, const pair<int,Path> &b){return a.second.pointSum<b.second.pointSum;}
        );
        int tmpWalls[MAX_SIDE][MAX_SIDE] = {};
        int cnt[MAX_AGENT] = {}, setCount=0;
        bool set__idx[MAX_AGENT] = {}, set__coord[MAX_SIDE][MAX_SIDE] = {};

        rep(i,h) rep(j,w) tmpWalls[i][j] = walls[i][j];
        for(auto a : v) {
            pq.push(a);
            cnt[a.first]++;
        }

        while(!pq.empty() && setCount != agentNum) {
            vector<pair<int, Vec2>> originalWallValue;
            const int idx = pq.top().first;
            const int point = pq.top().second.pointSum;
            const vector<Action> actions = pq.top().second.path;
            pq.pop();
            if (set__idx[idx]) continue;

            int actualPoint=0;
//            for(auto action : actions) {
            rep(i,min(3,(int)actions.size())) {
                Action action = actions[i];
                const Vec2 targetCoord = action.targetCoord;
                const int y = targetCoord.y, x = targetCoord.x;
                const string type = action.type;
                const int tmpPoint = points[y][x];

                originalWallValue.emplace_back(tmpWalls[y][x], targetCoord);

                if (type == MOVE) {
                    if (tmpWalls[y][x] == 0) {
                        if (areas[y][x] == teams[0].teamID) actualPoint += tmpPoint - abs(tmpPoint);
                        else if (areas[y][x] == teams[1].teamID) actualPoint += tmpPoint + abs(tmpPoint);
                        else if (tmpWalls[y][x] == 0) actualPoint += tmpPoint;
                    }
                    tmpWalls[y][x] = teams[0].teamID;
                }
                if (type == REMOVE) {
                    if (tmpWalls[y][x] == teams[0].teamID) actualPoint -= tmpPoint;
                    if (tmpWalls[y][x] == teams[1].teamID) actualPoint += tmpPoint;
                    tmpWalls[y][x] = 0;
                }
            }


            const int ty = actions.front().targetCoord.y, tx = actions.front().targetCoord.x;
            if ((actualPoint == point || cnt[idx] == 1) && !set__coord[ty][tx]) {
                teams[0].agents[idx].action = actions.front();
                setCount++;
                set__idx[idx] = true;
                set__coord[ty][tx] = true;
            } else {
                if (actualPoint != point) {
                    Path tmpPath;
                    tmpPath.pointSum = actualPoint;
                    tmpPath.path = actions;
                    pq.push({idx, tmpPath});
                } else cnt[idx]--;
                for(auto [value, coord] : originalWallValue) tmpWalls[coord.y][coord.x] = value;
            }
        }
    }
    output();
}

double calcAreaEV(Vec2 &topL, Vec2 &bottomR) {
    int imos[MAX_SIDE+2][MAX_SIDE+2]={}, absimos[MAX_SIDE+2][MAX_SIDE+2]={};
    rep1(i,h) rep1(j,w) {
        imos[i][j] = points[i-1][j-1];
        absimos[i][j] = abs(points[i-1][j-1]);
    }
    rep1(i,h) rep1(j,w-1) imos[i][j+1] += imos[i][j], absimos[i][j+1] += absimos[i][j];
    rep1(i,h-1) rep1(j,w) imos[i+1][j] += imos[i][j], absimos[i+1][j] += absimos[i][j];

    auto f = [](int p[MAX_SIDE+2][MAX_SIDE+2], Vec2 s, Vec2 t) {
        return p[t.y][t.x] + p[s.y-1][s.x-1] - p[t.y][s.x-1] - p[s.y-1][t.x];
    };
    double ev = -1;
    for(int i=1; i<=h-2; ++i) for(int j=1; j<=w-2; ++j) for(int I=i+2; I<=h; ++I) for(int J=j+2; J<=w; ++J) {
        if (I-i + J-j <= agentNum) {
            Vec2 s[2] = {Vec2(j,i), Vec2(j+1,i+1)}, t[2] = {Vec2(J,I), Vec2(J-1, I-1)};
            int tmpev = f(imos, s[0], t[0]) - f(imos, s[1], t[1]) + f(absimos, s[1], t[1]);
            if (ev < tmpev) {
                ev = tmpev;
                topL = Vec2(j-1, i-1);
                bottomR = Vec2(J-1, I-1);
            }
        }
    }
    ev /= 2.0*(bottomR.x - topL.x + bottomR.y - topL.y);
    return ev;
}