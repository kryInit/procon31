// wallsを直で書き換えている(再帰を抜ければ元通り)
// 困るなら適当な配列に代入してその配列を使って

#include <bits/stdc++.h>
#include "constant.hpp"
#include "classes.hpp"
#include "utility.hpp"

using namespace std;

#define rep(i,n) for(int i=0;i<(int)(n);++i)
#define rep1(i,n) for(int i=1;i<=(int)(n);++i)
#define rep11(i,n) for(int i=1;i<(int)(n);++i)
#define repo(i,o,n) for(int i=o;i<(int)(n);++i)
#define repm(i,n) for(int i=(int)(n)-1;i>=0;--i)

struct Parameters {
    const int maxDepth, maxRadix, depthAtEnumeration, maxCntOfMoveToZero;
};


extern int h, w, agentNum, turns, turn;
extern int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
extern Team teams[TEAM_NUM];


void __enumeratePath(vector<Path> &paths, Path &nowPath, Vec2 nowCoord, int nowDepth, const Parameters p) {
    if (nowDepth > p.maxDepth) return;

    const int d[3] = {-1, 0, 1};

    // 周辺9マスで取り得る選択肢を列挙, その行動で得られるスコアと共にcandidatesにpush_back
    vector<pair<int, Action>> candidates;
    rep(i,3) rep(j,3) {
        int x = nowCoord.x+d[i], y = nowCoord.y+d[j];
        if (!isSafeIdx(Vec2(x,y)) || (i == 1 && j == 1)) continue;
        Vec2 coord(x,y);
        if (areas[y][x] == teams[0].teamID) candidates.push_back({points[y][x]-abs(points[y][x]), Action(MOVE, coord)});
        else if (walls[y][x] == 0) candidates.push_back({points[y][x], Action(MOVE, coord)});
        else if (walls[y][x] == teams[1].teamID) candidates.push_back({points[y][x], Action(REMOVE, coord)});
        else if (walls[y][x] == teams[0].teamID) {
            candidates.push_back({0, Action(MOVE, coord)});
            candidates.push_back({-points[y][x], Action(REMOVE, coord)});
        }
    }
    // 行動で得られるスコアの降順にソート
    sort(candidates.begin(), candidates.end(), [](const pair<int,Action>& l, const pair<int, Action>& r){return l.first > r.first;});

    // スコアの高い順に再帰にかけていき, 最も良さげなpathを選んでbestPathに格納
    Path bestPath;
    int cntOfMoveToZero = 0;
    int n = min((int)candidates.size(), p.maxRadix);
    rep(i,n) {
        int point = candidates[i].first;
        Vec2 targetCoord = candidates[i].second.targetCoord;
        string type = candidates[i].second.type;
        int x = targetCoord.x, y = targetCoord.y;

        if (point == 0 && n < candidates.size()) {
            if (cntOfMoveToZero >= p.maxCntOfMoveToZero) {
                n++;
                continue;
            } else cntOfMoveToZero++;
        }

        Path tmpPath = nowPath;
        int originalWall = walls[y][x];
        Vec2 nextCoord;
        if (type == MOVE) {
            walls[y][x] = teams[0].teamID;
            nextCoord = targetCoord;
        }
        if (type == REMOVE) {
            walls[y][x] = 0;
            nextCoord = nowCoord;
        }
        tmpPath.path.push_back(candidates[i].second);
        tmpPath.pointSum += point;

        __enumeratePath(paths, tmpPath, nextCoord, nowDepth+1, p);

        walls[y][x] = originalWall;
        if (bestPath.pointSum < tmpPath.pointSum) bestPath = tmpPath;
    }

    if (nowDepth == p.depthAtEnumeration) paths.push_back(bestPath);

    nowPath = bestPath;
}

void enumeratePath(vector<pair<int,Path>> &paths, const int maxDepth, const int maxRadix, const int depthAtEnumeration, const int maxCntOfMoveToZero) {
    if (maxDepth < 1 || maxRadix < 1 || depthAtEnumeration < 0 || depthAtEnumeration > maxDepth) return;
    const Parameters p = {maxDepth, maxRadix, depthAtEnumeration, maxCntOfMoveToZero};

    for(int i=0; i<agentNum; ++i) {
        vector<Path> _paths;
        Path nowPath;
        __enumeratePath(_paths, nowPath, teams[0].agents[i].coord, 0, p);
        for(auto path : _paths) paths.push_back(make_pair(i, path));
    }
}
