// wallsを直で書き換えている(再帰を抜ければ元通り)
// 困るなら適当な配列に代入してその配列を使って

// simpleの実装は再帰を用いているが、無印の実装には再帰を用いていない
// 無印の方が若干高速

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

extern int h, w, agentNum, turns, turn;
extern int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
extern Team teams[TEAM_NUM];

extern const int MIN_UNTAKEN_WALL_POINT = 1;
extern const double MAGNIFICATION_OF_REMOVE = 1.3;

struct forRecursionData {
    int depth, originalWall, level, parentIdx;
    Vec2 coord;
    Path prevPath, bestPath;
    forRecursionData(int _depth, int _originalWall, int _level, int _parentIdx, Vec2 _coord, Path _prevPath) {
        depth = _depth;
        originalWall = _originalWall;
        level = _level;
        parentIdx = _parentIdx;
        coord = _coord;
        prevPath = _prevPath;
        bestPath = Path();
        bestPath.pointSum = -1e5;
    }
};


void enumeratePath(vector<pair<int,Path>> &paths, const int maxDepth, const int maxRadix, const int depthAtEnumeration, const int maxCntOfMoveToZero) {
    if (maxDepth < 1 || maxRadix < 1 || depthAtEnumeration < 0 || depthAtEnumeration > maxDepth) return;
    const int d[3] = {-1, 0, 1};

    for(int agentIdx=0; agentIdx<agentNum; ++agentIdx) {
        if (teams[0].agents[agentIdx].coord.x < 0 && teams[0].agents[agentIdx].coord.y < 0) continue;
        vector<forRecursionData> forRecursion;

        {
            Vec2 nowCoord = teams[0].agents[agentIdx].coord;
            forRecursion.emplace_back(0, walls[nowCoord.y][nowCoord.x], 0, -1, nowCoord, Path());
        }

        while(!forRecursion.empty()) {
            const forRecursionData tmpfrd = forRecursion.back();
            const int nowIdx = (int)forRecursion.size()-1;
            const int nowDepth = tmpfrd.depth;
            const int nowLevel = tmpfrd.level;
            const Vec2 nowCoord = tmpfrd.coord;
            const Path nowPath = tmpfrd.prevPath;

            if (nowLevel == 0) {
                // Actionを実行している(あれば)
                if (!nowPath.path.empty()) {
                    const Action lastAction = nowPath.path.back();
                    const Vec2 targetCoord = lastAction.targetCoord;
                    const string type = lastAction.type;
                    if (type == MOVE) walls[targetCoord.y][targetCoord.x] = teams[0].teamID;
                    if (type == REMOVE) walls[targetCoord.y][targetCoord.x] = 0;
                }
                forRecursion[nowIdx].level = 1;
                // 子(のいくつか, maxRadix以内)をforRecursionにpush_backしている
                if (nowDepth < maxDepth) {
                    // 周辺9マスで取り得る選択肢を列挙, その行動で得られるスコアと共にcandidatesにpush_back
                    vector<pair<int, Action>> candidates;
                    rep(i,3) rep(j,3) {
                        const int x = nowCoord.x+d[i], y = nowCoord.y+d[j];
                        if (!isSafeIdx(Vec2(x,y)) || (i == 1 && j == 1)) continue;
                        const Vec2 coord(x,y);
                        if (areas[y][x] == teams[0].teamID) candidates.emplace_back(points[y][x]-abs(points[y][x]), Action(MOVE, coord));
                        else if (areas[y][x] == teams[1].teamID) candidates.emplace_back(points[y][x]+abs(points[y][x]), Action(MOVE, coord));
                        else if (walls[y][x] == 0) candidates.emplace_back(max(MIN_UNTAKEN_WALL_POINT,points[y][x]), Action(MOVE, coord));
                        else if (walls[y][x] == teams[1].teamID) candidates.emplace_back(points[y][x]*MAGNIFICATION_OF_REMOVE, Action(REMOVE, coord));
                        else if (walls[y][x] == teams[0].teamID) {
                            candidates.emplace_back(0, Action(MOVE, coord));
                            candidates.emplace_back(-points[y][x]*MAGNIFICATION_OF_REMOVE, Action(REMOVE, coord));
                        }
                    }
                    // 行動で得られるスコアの降順にソート
                    sort(candidates.begin(), candidates.end(), [](const pair<int,Action>& l, const pair<int, Action>& r){return l.first > r.first;});

                    // スコアの高い順に再帰にかけていき, 最も良さげなpathを選んでbestPathに格納
                    int cntOfMoveToZero = 0;
                    int n = min((int)candidates.size(), maxRadix);
                    rep(i,n) {
                        const int point = candidates[i].first;
                        const Vec2 targetCoord = candidates[i].second.targetCoord;
                        const string type = candidates[i].second.type;
                        const int x = targetCoord.x, y = targetCoord.y;

                        if (point == 0 && n < (int)candidates.size()) {
                            if (cntOfMoveToZero >= maxCntOfMoveToZero) {
                                n++;
                                continue;
                            } else cntOfMoveToZero++;
                        }

                        Path tmpPath = nowPath;
                        int originalWall = walls[y][x];
                        Vec2 nextCoord (type == MOVE ? targetCoord : nowCoord);

                        tmpPath.path.push_back(candidates[i].second);
                        tmpPath.pointSum += point;

                        forRecursion.emplace_back(nowDepth+1, originalWall, 0, nowIdx, nextCoord, tmpPath);
                    }
                }
            } else { // equal to if (nowLevel == 1)
                // Actionの実行を戻している(実行していれば)
                if (!nowPath.path.empty()) {
                    const int y = nowPath.path.back().targetCoord.y;
                    const int x = nowPath.path.back().targetCoord.x;
                    walls[y][x] = tmpfrd.originalWall;
                }
                // bestPath(葉ならnowPath)をparentのbestPathに突っ込んでいる
                Path bestPath = (nowDepth == maxDepth ? nowPath : tmpfrd.bestPath);
                if (0 <= tmpfrd.parentIdx) forRecursion[tmpfrd.parentIdx].bestPath = bestPath;

                forRecursion.pop_back();

                if (nowDepth == depthAtEnumeration) paths.emplace_back(agentIdx, bestPath);
            }
        }
    }
}



struct Parameters {
    const int maxDepth, maxRadix, depthAtEnumeration, maxCntOfMoveToZero;
};

void __enumeratePath__simple(vector<Path> &paths, Path &nowPath, Vec2 nowCoord, int nowDepth, const Parameters p) {
    if (nowDepth > p.maxDepth) return;

    const int d[3] = {-1, 0, 1};

    // 周辺9マスで取り得る選択肢を列挙, その行動で得られるスコアと共にcandidatesにpush_back
    vector<pair<int, Action>> candidates;
    rep(i,3) rep(j,3) {
            int x = nowCoord.x+d[i], y = nowCoord.y+d[j];
            if (!isSafeIdx(Vec2(x,y)) || (i == 1 && j == 1)) continue;
            Vec2 coord(x,y);
            if (areas[y][x] == teams[0].teamID) candidates.push_back({points[y][x]-abs(points[y][x]), Action(MOVE, coord)});
            else if (areas[y][x] == teams[1].teamID) candidates.push_back({points[y][x]+abs(points[y][x]), Action(MOVE, coord)});
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

        if (point == 0 && n < (int)candidates.size()) {
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

        __enumeratePath__simple(paths, tmpPath, nextCoord, nowDepth+1, p);

        walls[y][x] = originalWall;
        if (bestPath.pointSum < tmpPath.pointSum) bestPath = tmpPath;
    }

    if (nowDepth == p.depthAtEnumeration) paths.push_back(bestPath);

    nowPath = bestPath;
}

void enumeratePath__simple(vector<pair<int,Path>> &paths, const int maxDepth, const int maxRadix, const int depthAtEnumeration, const int maxCntOfMoveToZero) {
    if (maxDepth < 1 || maxRadix < 1 || depthAtEnumeration < 0 || depthAtEnumeration > maxDepth) return;
    const Parameters p = {maxDepth, maxRadix, depthAtEnumeration, maxCntOfMoveToZero};

    for(int i=0; i<agentNum; ++i) {
        vector<Path> _paths;
        Path nowPath;
        __enumeratePath__simple(_paths, nowPath, teams[0].agents[i].coord, 0, p);
        for(auto path : _paths) paths.push_back(make_pair(i, path));
    }
}
