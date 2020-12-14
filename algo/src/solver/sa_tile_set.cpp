#include<bits/stdc++.h>
#include "utility.hpp"
#include "constant.hpp"
#include "classes.hpp"
using namespace std;

#define rep(i,n) for(int i=0;i<(int)(n);++i)
#define rep1(i,n) for(int i=1;i<=(int)(n);++i)
#define rep11(i,n) for(int i=1;i<(int)(n);++i)
#define repo(i,o,n) for(int i=o;i<(int)(n);++i)
#define repm(i,n) for(int i=(int)(n)-1;i>=0;--i)

unsigned Xseed[4];
void initXS(unsigned s) {
    for(int i=1; i<=4; i++){
        Xseed[i-1] = s = 1812433253 * (s^(s>>30)) + (unsigned)i;
    }
}
unsigned xs(void) {
    unsigned t;
    t=(Xseed[0]^(Xseed[0]<<11));
    Xseed[0]=Xseed[1];
    Xseed[1]=Xseed[2];
    Xseed[2]=Xseed[3];
    return Xseed[3]=(Xseed[3]^(Xseed[3]>>19))^(t^(t>>8));
}

void input();
void output();
void initialize(int argc, char *argv[]);
void finalize();

void calcDists();
Vec2 nextCoord(Vec2 now, Vec2 to);

int h, w, agentNum, turns, turn, operationMillis;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];


int matchID;
int dist[MAX_SIDE][MAX_SIDE][MAX_SIDE][MAX_SIDE];
int actualPoints[MAX_SIDE][MAX_SIDE];
int d[3] = {-1,0,1};
string dataPath;
clock_t start;


int main(int argc, char *argv[]) {
    initialize(argc, argv);

    finalize();
}

int calcDistWithPoint(Vec2 coord) {
    int y = coord.y, x = coord.x;
    int result;
    if (areas[y][x] == teams[0].teamID && points[y][x] < 0) result = 3;
    else if (areas[y][x] == teams[1].teamID) result = 1;
    else if (walls[y][x] == teams[0].teamID) result = 1;
    else if (walls[y][x] == teams[1].teamID) result = 2;
    else if (points[y][x] < 0) result = 2;
    else result = 1;
    return result;
}

void calcDists() {
    struct state {
        Vec2 to;
        int dist;
        state(){to=Vec2();dist=1e5;};
        state(Vec2 _to, int _dist) {
            to = _to;
            dist = _dist;
        }
        bool operator<( const state& another ) const {
            return dist > another.dist;
        }
    };
    const int d[3] = {-1,0,1};

#pragma omp parallel for
    rep(si,h) rep(sj,w) {
        priority_queue<state> pq;
        bool visited[MAX_SIDE][MAX_SIDE] = {};
        pq.push(state(Vec2(sj,si), 0));
        while(pq.size()) {
            int _dist=pq.top().dist;
            Vec2 now=pq.top().to;
            pq.pop();
            if (visited[now.y][now.x]) continue;
            visited[now.y][now.x] = true;
            dist[si][sj][now.y][now.x] = _dist;
            rep(i,3) rep(j,3) {
                Vec2 to(now.x+d[i], now.y+d[j]);
                if (isSafeIdx(to) && !visited[to.y][to.x]) {
                    int dDist = calcDistWithPoint(to);
                    pq.push(state(to, _dist+dDist));
                }
            }
        }
    }
}

Vec2 nextCoord(Vec2 now, Vec2 to) {
    if (abs(now.x-to.x) <= 1 && abs(now.y-to.y) <= 1) return to;
    struct state {
        Vec2 to,first;
        int dist;
        state(){to=first=Vec2();dist=1e5;};
        state(Vec2 _to, Vec2 _first, int _dist) {
            to = _to;
            first = _first;
            dist = _dist;
        }
        bool operator<( const state& another ) const {
            return dist == another.dist ? actualPoints[to.y][to.x] < actualPoints[another.to.y][another.to.x] : dist > another.dist;
        }
    };
    const int d[3] = {-1,0,1};
    priority_queue<state> pq;
    bool visited[MAX_SIDE][MAX_SIDE] = {};
    visited[now.y][now.x] = true;
    rep(i,3) rep(j,3) {
        Vec2 _to(now.x+d[i], now.y+d[j]);
        if (isSafeIdx(_to)) {
            int dDist = calcDistWithPoint(to);
            pq.push(state(_to, _to, dDist));
        }
    }
    while(pq.size()) {
        int _dist=pq.top().dist;
        Vec2 _now=pq.top().to;
        Vec2 first = pq.top().first;
        pq.pop();
        if (visited[_now.y][_now.x]) continue;
        if (_now == to) return first;
        visited[_now.y][_now.x] = true;
        rep(i,3) rep(j,3) {
            Vec2 _to(_now.x+d[i], _now.y+d[j]);
            if (isSafeIdx(_to) && !visited[_to.y][_to.x]) {
                int dDist = calcDistWithPoint(to);
                pq.push(state(_to, first, _dist+dDist));
            }
        }
    }
}

void initialize(int argc, char *argv[]) {
    start = clock();
    if (argc < 2) {
        cerr << "引数が足りません" << endl;
        exit(1);
    }

    initXS(time(NULL));

    matchID = atoi(argv[1]);
    dataPath = (string)(((filesystem::path)(string(argv[0]))).parent_path()) + "/../../data/" + to_string(matchID) + "/";
    input();
    calcDists();

    rep(i,h) rep(j,w) {
        int point = 0;
        if (areas[i][j] == teams[0].teamID) point = points[i][j] - abs(points[i][j]);
        else if (areas[i][j] == teams[1].teamID) point = points[i][j] + abs(points[i][j]);
        else if (walls[i][j] == teams[0].teamID) point = max(0, -points[i][j]);
        else point = points[i][j];
        actualPoints[i][j] = point;
    }

#ifdef DEBUG
    cout << "solver!" << endl;
    operationMillis = 3000;
#endif

}
void finalize() {
    rep(i,agentNum) teams[1].agents[i].action = Action();
    output();
}
