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


struct BestPath{
    int dist, score;
    Action nextAction;
    BestPath() {
        dist = score = 0;
        nextAction = Action();
    }
    BestPath(int _dist, int _score, Action _nextAction) {
        dist = _dist;
        score = _score;
        nextAction = _nextAction;
    }
};

void input();
void output();

void initialize(int argc, char *argv[]);
void finalize();

void predictionEnemyAction(); // 敵の行動を予測する。ある程度の予測ができるものはteams[1].agents[i].actionに代入される。
void initialSet(vector<Vec2> tileSet[MAX_AGENT]); // タイルのセットの初期値を作成する。
void changeSet(vector<Vec2> tileSet[MAX_AGENT]); // タイルのセットを変更する。(この部分の乱択でスコアを上げてみる
void calcBestPath(vector<Vec2> tileSet[MAX_AGENT], BestPath bestPath[MAX_AGENT]); // タイルのセットを全部通る最も良い(と思われる)Pathのdistとscoreをメモる。
double calcExpectedScore(BestPath bestPath[MAX_AGENT], Action nextAction[MAX_AGENT]); // bestPathからexpectedScoreを計算する。

int h, w, agentNum, turns, turn, operationMillis;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];

int matchID;
int dist[MAX_SIDE][MAX_SIDE][MAX_SIDE][MAX_SIDE];
int actualPoints[MAX_SIDE][MAX_SIDE];
int d[3] = {-1,0,1};
string dataPath;
clock_t start;

const int MAX_SET_SIZE = 5;

int main(int argc, char *argv[]) {
    initialize(argc, argv);

    double nowMaxExpectedScore = 0;
    vector<Vec2> tileSet[MAX_AGENT];
    predictionEnemyAction();
    initialSet(tileSet);
    const int border = min(CLOCKS_PER_SEC, (operationMillis - 1000) * (CLOCKS_PER_SEC / 1000));
    int loopCount=0;
    while(clock() - start < border) {
        loopCount++;
        vector<Vec2> tmpTileSet[MAX_AGENT]; rep(i,agentNum) tmpTileSet[i] = tileSet[i];
        double tmpScore = 0;
        BestPath bestPath[MAX_AGENT];
        Action nextAction[MAX_AGENT];

        changeSet(tileSet);
        calcBestPath(tileSet, bestPath);
        tmpScore = calcExpectedScore(bestPath, nextAction);

        if (nowMaxExpectedScore < tmpScore) {
            nowMaxExpectedScore = tmpScore;
            rep(i,agentNum) teams[0].agents[i].action = nextAction[i];
        } else if (nowMaxExpectedScore*0.8 > tmpScore) rep(i,agentNum) tileSet[i] = tmpTileSet[i]; // 現象の割合がある一定以下であれば戻さないとかしても良いかも
    }
    rep(i,agentNum) if (teams[0].agents[i].coord.x < 0) {
        teams[0].agents[i].action = Action(SET, Vec2((w/4) * (i/4)+3 , (h/4) * (i%4)+3));
    }
    cout << endl;
    cerr << loopCount << endl;
    finalize();
}

void predictionEnemyAction() {
    rep(i,agentNum) {
        int y = teams[1].agents[i].coord.y, x = teams[1].agents[i].coord.x;
        pair<int,Vec2> highPoints[2]; highPoints[0] = highPoints[1] = make_pair(-32, Vec2());
        rep(j,3) rep(k,3) {
            int Y = y+d[j], X = x+d[k];
            Vec2 coord = Vec2(X,Y);
            if (isSafeIdx(coord)) {
                int point = 0;
                if (areas[Y][X] == teams[0].teamID) point = points[Y][X] + abs(points[Y][X]);
                else if (areas[Y][X] == teams[1].teamID) point = points[Y][X] - abs(points[Y][X]);
                else if (walls[Y][X] == teams[1].teamID) point = max(0, -points[Y][X]);
                else point = points[Y][X];
                if (highPoints[0].first < point) {
                    highPoints[1] = highPoints[0];
                    highPoints[0] = make_pair(point, coord);
                }
                else if (highPoints[1].first < point) {
                    highPoints[1] = make_pair(point, coord);
                }
            }
        }
        Vec2 coord = highPoints[0].second;
        int Y = coord.y, X = coord.x;
        if (highPoints[1].first < 0 || highPoints[1].first + 6 < highPoints[0].first) {
            string type = (walls[Y][X] == teams[0].teamID || (walls[Y][X] == teams[1].teamID && points[Y][X] < 0)) ? REMOVE : MOVE;
            teams[1].agents[i].action = Action(type, coord);
        } else if ((walls[Y][X] == teams[0].teamID || (walls[Y][X] == teams[1].teamID && points[Y][X] < 0))) {
            teams[1].agents[i].action = Action(REMOVE, coord);
        }
    }
}

void initialSet(vector<Vec2> tileSet[MAX_AGENT]) {
    bool takenTile[MAX_SIDE][MAX_SIDE]={};
    vector<pair<float, Vec2>> v[MAX_AGENT];
    rep(k,agentNum) {
        int y = teams[0].agents[k].coord.y, x = teams[0].agents[k].coord.x;
        if (!isSafeIdx(Vec2(x,y))) continue;
        rep(i,h) rep(j,w) {
            if (i == y && j == x) continue;
            v[k].emplace_back((float)actualPoints[i][j]/dist[y][x][i][j], Vec2(j,i));
        }
        sort(v[k].begin(), v[k].end(), [](auto const& l, auto const& r) {
            return l.first < r.first;
        });
#ifdef DEBUG
        rep(i,10) {
            auto tmp = v[k][v[k].size()-i-1];
            cout << "(" << actualPoints[tmp.second.y][tmp.second.x] << ", " << dist[y][x][tmp.second.y][tmp.second.x] << ", (" << tmp.second.y << ", " << tmp.second.x << "), " << tmp.first << "), ";
        }
        cout << "\n" << endl;
#endif
    }
    rep(_,MAX_SET_SIZE) {
        rep(i,agentNum) {
            while(!v[i].empty()) {
                int y = v[i].back().second.y, x = v[i].back().second.x;
                if (!takenTile[y][x]) break;
                v[i].pop_back();
            }
            if (v[i].empty()) break;
            tileSet[i].push_back(v[i].back().second);
            takenTile[v[i].back().second.y][v[i].back().second.x] = true;
            v[i].pop_back();
        }
    }
#ifdef DEBUG
    char c[MAX_SIDE][MAX_SIDE] = {};
    rep(i,h) rep(j,w) c[i][j] = '.';
    rep(i,agentNum) {
        for(auto j : tileSet[i]) c[j.y][j.x] = 'a'+i;
        if (teams[0].agents[i].coord.x >= 0) c[teams[0].agents[i].coord.y][teams[0].agents[i].coord.x] = 'A'+i;
    }
    rep(i,h) {
        rep(j,w) cout << c[i][j] << " ";
        cout << endl;
    }
#endif
}

void changeSet(vector<Vec2> tileSet[MAX_AGENT]) {
    rep(i,agentNum) if (!tileSet[i].empty()) {
        while(true) {
            int mode = xs()%3;
            if (mode == 0) { // tileSetの追加
                int idx = xs()%agentNum;
                if (tileSet[idx].size() < MAX_SET_SIZE) continue;
                tileSet[idx].push_back(Vec2(xs()%w, xs()%h));
            } else if (mode == 1) { // tileSetの削除
                int idx = xs()%agentNum;
                if (tileSet[idx].empty()) continue;
                int th = xs()%tileSet[idx].size();
                tileSet[idx].erase(tileSet[idx].begin()+th);
            } else { // tileSetのswap
                int idx[2]={};
                while(idx[0] == idx[1]) {
                    idx[0] = xs()%agentNum;
                    idx[1] = xs()%agentNum;
                }
                if (tileSet[idx[0]].empty() || tileSet[idx[1]].empty()) continue;
                int th[2]={};
                th[0] = xs()%tileSet[idx[0]].size();
                th[1] = xs()%tileSet[idx[1]].size();
                swap(tileSet[idx[0]][th[0]], tileSet[idx[0]][th[0]]);
            }
            break;
        }
        break;
    }
}

void calcBestPath(vector<Vec2> tileSet[MAX_AGENT], BestPath bestPath[MAX_AGENT]) {
    rep(l,agentNum) {
        if (tileSet[l].empty()) {
            bestPath[l] = BestPath(0,0,Action(MOVE, teams[0].agents[l].coord));
            continue;
        }
        int y = teams[0].agents[l].coord.y, x = teams[0].agents[l].coord.x;

        const int n = tileSet[l].size();
        const int N = (1<<n);
        vector<vector<pair<int,int>>> dp(n,vector<pair<int,int>>(N, make_pair(INT_MAX,-1)));
        rep(i,n) dp[i][(1<<i)] = make_pair(dist[y][x][tileSet[l][i].y][tileSet[l][i].x], i);
        rep(i,N) rep(j,n) if (i&(1<<j)) rep(k,n) {
            Vec2 from = tileSet[l][j], to = tileSet[l][k];
            if (dp[k][i|(1<<k)].first > dp[j][i].first+dist[from.y][from.x][to.y][to.x]) {
                dp[k][i|(1<<k)] = dp[j][i];
                dp[k][i|(1<<k)].first += dist[from.y][from.x][to.y][to.x];
            }
        }

        int pointSum = 0;
        int minDist=INT_MAX;
        int firstDirIdx=-1;
        for(auto i : tileSet[l]) pointSum += actualPoints[i.y][i.x];
        rep(i,n) {
            if (minDist > dp[i][N-1].first) {
                minDist = dp[i][N-1].first;
                firstDirIdx = dp[i][N-1].second;
            }
        }
        Vec2 nextTile = tileSet[l][firstDirIdx];
        vector<pair<pair<int,int>,Vec2>> v;
        rep(i,3) rep(j,3) {
            if (i == 1 && j == 1) continue;
            int Y = y+d[i], X = x+d[j];
            if (isSafeIdx(Vec2(X,Y))) {
                v.emplace_back(make_pair(dist[y][x][Y][X] + dist[Y][X][nextTile.y][nextTile.x], -actualPoints[Y][X]), Vec2(X,Y));
            }
        }
        sort(v.begin(), v.end());

        Vec2 nextCoord = v[0].second;
        string type;
        if (walls[nextCoord.y][nextCoord.x] == teams[1].teamID) type = REMOVE;
        else if (nextCoord == nextTile && walls[nextCoord.y][nextCoord.x] == teams[0].teamID && actualPoints[nextTile.y][nextTile.x] < 0) type = REMOVE;
        else type = MOVE;

        bestPath[l] = BestPath(minDist, pointSum, Action(type, nextCoord));
    }
}

double calcExpectedScore(BestPath bestPath[MAX_AGENT], Action nextAction[MAX_AGENT]) {

    int sumDist=0, sumScore=0;
    rep(i,agentNum) {
        sumDist = bestPath[i].dist;
        sumScore = bestPath[i].score;
        nextAction[i] = bestPath[i].nextAction;
    }

    return sumScore / (sumDist+1);
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
