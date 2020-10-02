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

int main(int argc, char *argv[]) {
    if (argc < 2) return 0;

    matchID = atoi(argv[1]);
    dataPath = (string)(((filesystem::path)(string(argv[0]))).parent_path()) + "/../../data/" + to_string(matchID) + "/";

    input();

    if (turn <= 1) {
        Vec2 topL, bottomR;
        double ev = calcAreaEV(topL, bottomR);
        cerr << ev << endl;
        if (ev > 0) {
            if (turn == 0) {
                Vec2 tmp = topL;
                rep(i,agentNum) {
                    teams[0].agents[i].action = Action(SET, tmp);
                    rep(_,2) {
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
            } else {
                Vec2 tmp = topL;
                rep(i,agentNum) {
                    rep(j,2) {
                        if (j == 1) teams[0].agents[i].action = Action(MOVE, tmp);
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
        }
    } else {
        const int maxDepth = 6;
        const int maxRadix = 5;
        const int depthAtEnumeration = 0;
        const int maxCntOfMoveToZero = 2;
        vector<pair<int,Path>> v;
        clock_t start = clock();
        cout << "start!" << endl;
        enumeratePath(v, maxDepth, maxRadix, depthAtEnumeration, maxCntOfMoveToZero);
        cout << (double)(clock()-start) / CLOCKS_PER_SEC << "[s]" << endl;
        cout << v.size() << endl;
        rep(i,v.size()) {
            if (i == 0) {
                vector<Action> path = v[i].second.path;
                for(auto action : path) {
                    cout << action.type << " (" << action.targetCoord.x << ", " << action.targetCoord.y << ") ->" << endl;
                }
            }
            int point = v[i].second.pointSum;
            Action action = v[i].second.path.front();
            teams[0].agents[v[i].first].action = v[i].second.path.front();
            cout << point << " " << action.type
                 << " (" << teams[0].agents[v[i].first].coord.x << ", " << teams[0].agents[v[i].first].coord.y << ") -> ("
                 << action.targetCoord.x << ", " << action.targetCoord.y << ")" << endl;
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