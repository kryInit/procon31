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

void input();
void output();
void calcDists();
Vec2 nextCoord(Vec2 now, Vec2 to);

int h, w, agentNum, turns, turn, operationMillis;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];

int dist[MAX_SIDE][MAX_SIDE][MAX_SIDE][MAX_SIDE];

int matchID;
string dataPath;

random_device rd;

struct Route{
    int approxDist;
    int expectedScore;
    vector<Vec2> route;
    bool frontCanBeChanged, pause;

    Route() {
        approxDist = 0;
        expectedScore = 0;
        frontCanBeChanged = true;
        pause = false;
    }
    Route(Vec2 coord, bool isNOTPlaced) {
        approxDist = 0;
        expectedScore = (isNOTPlaced ? calcExpectedScore(coord) : 0);
        route.push_back(coord);
        frontCanBeChanged = isNOTPlaced;
        pause = false;
    }


    // return (idx, (expected score, diff dist))
    pair<int,pair<int,int>> insertES(Vec2 target) {
#ifdef DEBUG
        if (route.empty()) {
            cerr << "ERROR: path size is 0." << endl;
            exit(0);
        }
#endif
        int es = calcExpectedScore(target);
        int minDist = INT_MAX;
        int idx = 0;
        rep(i,route.size()+1) {
            int tmpDist = INT_MAX;
            if (i == 0) {
                if (frontCanBeChanged) tmpDist = dist[target.y][target.x][route[i].y][route[i].x];
            } else if (i == route.size()) {
                tmpDist = dist[route[i-1].y][route[i-1].x][target.y][target.x];
            } else {
                tmpDist = dist[route[i-1].y][route[i-1].x][target.y][target.x] +
                          dist[target.y][target.x][route[i].y][route[i].x] -
                          dist[route[i-1].y][route[i-1].x][route[i].y][route[i].x];
            }
            if (minDist > tmpDist) {
                minDist = tmpDist;
                idx = i;
            }
        }
        return make_pair(idx, make_pair(es, minDist));
    }
    // return (expected score, diff dist)
    pair<int,int> deleteES(int idx) {
#ifdef DEBUG
        if (route.empty()) {
            cerr << "ERROR: delete target doesn't exists" << endl;
            exit(0);
        }
#endif
        if (idx == 0 && !frontCanBeChanged) return make_pair(0,0);
        int es = -calcExpectedScore(route[idx]);
        int dDist = 0;
        if (idx == 0) {
            dDist = -dist[route[idx].y][route[idx].x][route[idx+1].y][route[idx+1].x];
        } else if (idx == route.size()-1) {
            dDist = -dist[route[idx-1].y][route[idx-1].x][route[idx].y][route[idx].x];
        } else {
            dDist = dist[route[idx-1].y][route[idx-1].x][route[idx+1].y][route[idx+1].x] -
                    dist[route[idx-1].y][route[idx-1].x][route[idx].y][route[idx].x] -
                    dist[route[idx].y][route[idx].x][route[idx+1].y][route[idx+1].x];
        }
        return make_pair(es, dDist);
    }
    void insert(Vec2 target, int idx) {
        // procにactionをinsertして、approxDistとexpectedScoreを更新する
        auto itr = route.begin() + idx;
        int dDist = 0;
        if (idx == 0) {
            if (frontCanBeChanged) dDist = dist[target.y][target.x][route[idx].y][route[idx].x];
        } else if (idx == route.size()) {
            dDist = dist[route[idx-1].y][route[idx-1].x][target.y][target.x];
        } else {
            dDist = dist[route[idx-1].y][route[idx-1].x][target.y][target.x] +
                      dist[target.y][target.x][route[idx].y][route[idx].x] -
                      dist[route[idx-1].y][route[idx-1].x][route[idx].y][route[idx].x];
        }
        approxDist += dDist;
        expectedScore += calcExpectedScore(target);
        route.insert(itr, target);
    }
    void _delete(int idx) {
        if (idx == 0 && !frontCanBeChanged) return;
        int dDist = 0;
        if (idx == 0) {
            dDist = -dist[route[idx].y][route[idx].x][route[idx+1].y][route[idx+1].x];
        } else if (idx == route.size()-1) {
            dDist = -dist[route[idx-1].y][route[idx-1].x][route[idx].y][route[idx].x];
        } else {
            dDist = dist[route[idx-1].y][route[idx-1].x][route[idx+1].y][route[idx+1].x] -
                    dist[route[idx-1].y][route[idx-1].x][route[idx].y][route[idx].x] -
                    dist[route[idx].y][route[idx].x][route[idx+1].y][route[idx+1].x];
        }
        approxDist += dDist;
        expectedScore += -calcExpectedScore(route[idx]);
        route.erase(route.begin() + idx);
    }
    void update() {
        // routeをいい感じに更新する。
        if (route.size() < 2) return;
        rep(i,route.size()) {
            auto tmp = deleteES(i);
            if (tmp.first > 0) _delete(i), i = 0;
        }
    }
    double objective() {
        if (approxDist == 0) return 0;
        return (double)expectedScore / approxDist;
    }

private:
    int calcExpectedScore(Vec2 coord) {
        int y = coord.y, x = coord.x;
        if (areas[y][x] == teams[0].teamID) return points[y][x] - abs(points[y][x]);
        else if (areas[y][x] == teams[1].teamID) return points[y][x] + abs(points[y][x]);
        else if (walls[y][x] == teams[0].teamID) return 0;
        else return points[y][x];
    }
};

struct BuildRoutes {
    Route routes[MAX_AGENT];

    BuildRoutes() {
        // 特殊な行動をさせる場合はここに関数をねじ込む。そしてpauseにtrueを入れる。
//        assignGreedyActionToAgentCloseToEnemy();
        rep(i,agentNum) {
            if (routes[i].pause) continue;
            if (teams[0].agents[i].coord.x < 0) {
                int tmp = (h*w)/agentNum * (i+0.4);
                routes[i] = Route(Vec2(tmp%w, tmp/w), true);
            } else {
                routes[i] = Route(teams[0].agents[i].coord, false);
            }
        }
    }

    void build() {
        double t = (operationMillis - 1000.) / 2000.;
        clock_t start = clock();
        bool pushed[MAX_SIDE][MAX_SIDE] = {};
        int loopCount = 0;
        int failCount = 0;
        rep(i,agentNum) if (!routes[i].pause) pushed[routes[i].route[0].y][routes[i].route[0].x] = true;
        rep(i,h) rep(j,w) if (areas[i][j] == teams[0].teamID || (walls[i][j] == teams[0].teamID && points[i][j] >= 0)) pushed[i][j] = true;
        cout << "5" << endl;
        while((clock() - start) < t*CLOCKS_PER_SEC) {
            loopCount++;
            if (loopCount%50 == 0) {
                rep(i,agentNum) if (!routes[i].pause) routes[i].update();
            }
            int idx = getIdxWithMinDist();
            if (idx < 0) break;
            Vec2 target = Vec2(-1,-1);
            double nowMaxAve = INT_MIN;
            int targetIdx = 0;
            rep(i,h) rep(j,w) if (!pushed[i][j]) {
                auto tmp = routes[idx].insertES(Vec2(j,i));
                double tmpAve = ((routes[idx].approxDist + tmp.second.second) == 0 ? 0 : (routes[idx].expectedScore + tmp.second.first) / (routes[idx].approxDist + tmp.second.second));
                if (nowMaxAve < tmpAve) {
                    nowMaxAve = tmpAve;
                    target = Vec2(j,i);
                    targetIdx = tmp.first;
                }
            }
            if (nowMaxAve > routes[idx].objective()) {
                routes[idx].insert(target, targetIdx);
//                cout << routes[idx].expectedScore << " " << routes[idx].approxDist << endl;
                pushed[target.y][target.x] = true;
            } else if (routes[idx].route.size() < 5 && nowMaxAve > 2) {
                routes[idx].insert(target, targetIdx);
//                cout << routes[idx].expectedScore << " " << routes[idx].approxDist << endl;
                pushed[target.y][target.x] = true;
            } else if (routes[idx].route.size() == 1) {
                routes[idx] = Route(Vec2(rd()%w, rd()%h), true);
            } else {
                failCount++;
                if (failCount > 30) break;
            }
        }
        cout << "loop count: " << loopCount << endl;
    }

private:
    int getIdxWithMinDist() {
        vector<pair<int,int>> v;
        rep(i,agentNum) if (!routes[i].pause) {
            v.emplace_back(routes[i].approxDist, i);
        }
        if (v.empty()) return -1;
        sort(v.begin(), v.end());
        return v[rd()%min(3,(int)v.size())].second;
    }
    // 敵がすごく密集している時には使わない方が良い。それ以外であれば使っても良い。
    void assignGreedyActionToAgentCloseToEnemy() {
        vector<pair<int,Vec2>> v[MAX_AGENT];
        int d[3] = {-1,0,1};
        int cnt = 0;
        rep(i,agentNum) {
            if (teams[0].agents[i].coord.x < 0) continue;
            int y = teams[0].agents[i].coord.y, x = teams[0].agents[i].coord.x;
            rep(j,agentNum) {
                Vec2 tmp = teams[1].agents[j].coord;
                if (max(abs(y - tmp.y), abs(x - tmp.x)) <= 2) {
                    cnt++;
                    rep(k,3) rep(l,3) {
                        if (k == 1 && l == 1) continue;
                        int Y = y+d[k], X = x+d[l];
                        if (!isSafeIdx(Vec2(X,Y))) continue;
                        int point = 0;
                        if (areas[Y][X] == teams[0].teamID) point = points[Y][X] - abs(points[Y][X]);
                        else if (areas[Y][X] == teams[1].teamID) point = points[Y][X] + abs(points[Y][X]);
                        else if (walls[Y][X] == teams[0].teamID) point = max(0, -points[Y][X]);
                        else point = points[Y][X];
                        v[i].push_back(make_pair(point,Vec2(X,Y)));
                    }
                    break;
                }
            }
        }
        rep(i,agentNum) sort(v[i].rbegin(), v[i].rend());
        bool forbiddenTile[MAX_SIDE][MAX_SIDE] = {};
        bool acted[MAX_SIDE] = {};
        rep(_,cnt) {
            int idx = -1, now_min = INT_MAX;
            rep(i,agentNum) {
                if (acted[i] || v[i].empty()) continue;
                int tmpSum=0, tmpCnt=0;
                for(auto j : v[i]) {
                    if (forbiddenTile[j.second.y][j.second.x]) continue;
                    tmpSum += j.first;
                    tmpCnt++;
                    if (tmpCnt == 3) continue;
                }
                if (now_min > tmpSum) {
                    now_min = tmpSum;
                    idx = i;
                }
            }
            for(auto i : v[idx]) {
                if (forbiddenTile[i.second.y][i.second.x]) continue;
                Vec2 coord = i.second;
                string type;
                if (walls[coord.y][coord.x] == teams[1].teamID || (walls[coord.y][coord.x] == teams[0].teamID && points[coord.y][coord.x] < 0)) type = REMOVE;
                else type = MOVE;
                teams[0].agents[idx].action = Action(type, coord);
                routes[idx].pause = true;
                forbiddenTile[coord.y][coord.x] = true;
                points[coord.y][coord.x] = 0;
                break;
            }
            acted[idx] = true;
        }
    }
};


int main(int argc, char *argv[]) {
    if (argc < 2) return 0;

    matchID = atoi(argv[1]);
    dataPath = (string)(((filesystem::path)(string(argv[0]))).parent_path()) + "/../../data/" + to_string(matchID) + "/";

    input();

#ifdef DEBUG
    cout << "solver!" << endl;
    operationMillis = 3000;
#endif

    calcDists();
    BuildRoutes br;
    br.build();
    rep(i,agentNum) {
        cout << i << ": " << br.routes[i].expectedScore << " " << br.routes[i].approxDist << " " << br.routes[i].objective() << endl;
        for(auto j : br.routes[i].route) cout << "(" << j.x << ", " << j.y << "), ";
        cout << endl;
    }
    cout << operationMillis << endl;

    char c[MAX_SIDE][MAX_SIDE] = {};
    rep(i,h) rep(j,w) c[i][j] = '.';
    rep(i,agentNum) {
        for(auto j : br.routes[i].route) {
            c[j.y][j.x] = 'a'+i;
        }
        if (!br.routes[i].pause) c[br.routes[i].route[0].y][br.routes[i].route[0].x] = 'A'+i;
    }
    rep(i,h) {
        rep(j,w) cout << setw(4) << c[i][j];
        cout << endl;
    }

    bool targeted[MAX_SIDE][MAX_SIDE] = {};
    Action actions[MAX_AGENT];
    rep(i,agentNum) {
        if (br.routes[i].pause) continue;
        if (teams[0].agents[i].coord.x < 0) {
            actions[i] = Action(SET, br.routes[i].route.front());
        } else if (br.routes[i].route.size() > 1){
            Vec2 target = nextCoord(teams[0].agents[i].coord, br.routes[i].route[1]);
            if (walls[target.y][target.x] == teams[1].teamID) actions[i] = Action(REMOVE, target);
            else actions[i] = Action(MOVE, target);
        } else {
            // 何らか
            actions[i] = Action(MOVE, teams[0].agents[i].coord);
            cout << "hmmmmmmmm?" << endl;
        }
        targeted[actions[i].targetCoord.y][actions[i].targetCoord.x] = true;
    }
    int d[3] = {-1,0,1};
    rep(i,agentNum) {
        if (br.routes[i].pause) continue;
        if (actions[i].type == MOVE) {
            int y = teams[0].agents[i].coord.y, x = teams[0].agents[i].coord.x;
            int nowMax = INT_MIN;
            Vec2 target;
            rep(j,3) rep(k,3) {
                if (j == 1 && k == 1) continue;
                int Y = y+d[j], X = x+d[k];
                if (!isSafeIdx(Vec2(X,Y))) continue;
                int point = INT_MIN;
                if (walls[Y][X] == teams[0].teamID) point = -points[Y][X];
                if (walls[Y][X] == teams[1].teamID) point = points[Y][X];
                if (nowMax < point && !targeted[Y][X]) {
                    nowMax = point;
                    target = Vec2(X,Y);
                }
            }
            int border = br.routes[i].objective();
            if (nowMax > border) {
                targeted[actions[i].targetCoord.y][actions[i].targetCoord.x] = false;
                actions[i] = Action(REMOVE, target);
                targeted[target.y][target.x] = true;
            }
        }
    }
    rep(i,agentNum) {
        cout << actions[i].type << " (" << actions[i].targetCoord.x << ", " << actions[i].targetCoord.y << ")" << endl;
        if (!br.routes[i].pause) teams[0].agents[i].action = actions[i];
    }
    output();
}

int calcDistWithPoint(Vec2 coord) {
    int y = coord.y, x = coord.x;
    int result;
    if (areas[y][x] == teams[1].teamID) result = 1;
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
            return dist > another.dist;
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
