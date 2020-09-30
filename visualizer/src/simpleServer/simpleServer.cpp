#include<bits/stdc++.h>
using namespace std;

#define rep(i,n) for(int (i)=0; (i)<(n); ++(i))

const int MAX_SIDE = 24;
const int MIN_SIDE = 12;
const int MAX_AGENT = 14;
const int MIN_AGENT = 6;
const int TEAM_NUM = 2;

const string MOVE = "move";
const string REMOVE = "remove";
const string SET = "set";

struct Vec2 {
    int x,y;
    bool operator<( const Vec2& another ) const {
        return x == another.x ? y < another.y : x < another.x;
    }
};
struct Action { Vec2 targetCoord; string type=""; };
struct Agent { int agentID; Vec2 coord; Action action; };
struct Team { int teamID, wallPoint, areaPoint; Agent agents[MAX_AGENT]; };

void init();
void initAction();
void input();
void outputGameInfo();
void outputFieldInfo();
void transition();
void paintArea();
void calcScore();

int h, w, agentNum, turns, turn;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];
string parentPath;


int main(int argc, char *argv[]) {
    parentPath = ((filesystem::path)(string(argv[0]))).parent_path();
    if (argc < 2) {
        cout << "few argument" << endl;
        return 0;
    }
    if (string(argv[1]) == "gen") {
        init();
        outputGameInfo();
        outputFieldInfo();
        initAction();
    }
    if (string(argv[1]) == "transition") {
        input();
        if (turn < turns) {
            transition();
            paintArea();
            calcScore();
            outputFieldInfo();
            initAction();
        }
    }
}

void init() {
    random_device rd;
    h = rd()%13 + 12;
    w = rd()%13 + 12;
    turns = rd()%71 + 30;
    
    rep(i,h) rep(j,w) points[i][j] = rd()%33 -16;
    agentNum = rd()%9 + 6;

    // 本来teamIDとagentIDは被っても良いはずであるが、簡単のため一つのmapで管理している。
    map<int, bool> isUsedID;
    rep(i,TEAM_NUM) {
        int teamID = rd()%100000;
        while(isUsedID[teamID]) teamID = rd()%100000;
        teams[i].teamID = teamID;
        rep(j,agentNum) {
            int agentID = rd()%100000;
            while(isUsedID[agentID]) agentID = rd()%100000;
            isUsedID[agentID] = true;
            teams[i].agents[j] = {agentID, {-1, -1}};
        }
    }
}

void initAction() {
    {
        ofstream ofs(parentPath + "/../data/0/action");
        ofs << 0 << endl;
    }
    {
        ofstream ofs(parentPath + "/../data/0/forcedAction");
        ofs << 0 << endl;
    }
}

void input() {
    {
        int _;
        ifstream ifs(parentPath + "/../data/0/gameInfo");
        ifs >> _ >> _ >> turns;
    }
    {
        int _;
        ifstream ifs(parentPath + "/../data/0/fieldInfo");
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
    {
        ifstream ifs(parentPath + "/../data/0/action");
        int actionNum;
        ifs >> actionNum;
        rep(i,actionNum) {
            int agentID,x,y;
            string type;
            ifs >> agentID >> x >> y >> type;
            Action action = {{x,y}, type};
            rep(j,TEAM_NUM) rep(k,agentNum) if (teams[j].agents[k].agentID == agentID) teams[j].agents[k].action = action;
        }
    }
    {
        ifstream ifs(parentPath + "/../data/0/forcedAction");
        int actionNum;
        ifs >> actionNum;
        rep(i,actionNum) {
            int agentID,x,y;
            string type;
            ifs >> agentID >> x >> y >> type;
            Action action = {{x,y}, type};
            rep(j,TEAM_NUM) rep(k,agentNum) if (teams[j].agents[k].agentID == agentID) teams[j].agents[k].action = action;
        }
    }
}
void outputGameInfo() {
    ofstream ofs(parentPath + "/../data/0/gameInfo");
    ofs << 0 << endl;
    ofs << teams[0].teamID << endl;
    ofs << turns << endl;
    ofs << "0\n0" << endl;
}
void outputFieldInfo() {
    ofstream ofs(parentPath + "/../data/0/fieldInfo");
    ofs << h << " " << w << " " << turn << " " << 0 << endl;
    rep(i,h) {
        rep(j,w) ofs << walls[i][j] << " ";
        ofs << endl;
    }
    rep(i,h) {
        rep(j,w) ofs << areas[i][j] << " ";
        ofs << endl;
    }
    rep(i,h) {
        rep(j,w) ofs << points[i][j] << " ";
        ofs << endl;
    }
    
    rep(i,2) {
        Team team = teams[i];
        ofs << team.teamID << " " << agentNum << endl;
        ofs << team.areaPoint << " " << team.wallPoint << endl;
        rep(j,agentNum) {
            Agent agent = team.agents[j];
            ofs << agent.agentID << " " << agent.coord.x << " " << agent.coord.y << endl;
        }
    }
}

void transition() {
    ++turn;
    bool f = true;
    while(f) {
        f = false;
        map<Vec2, int> conflict;
        rep(i,TEAM_NUM) {
            Team team = teams[i];
            rep(j,agentNum) {
                Agent agent = team.agents[j];
                conflict[agent.coord]++;
                if (!agent.action.type.empty()) {
                    conflict[agent.action.targetCoord]++;
                }
            }
        }
        rep(i,TEAM_NUM) {
            Team team = teams[i];
            rep(j,agentNum) {
                Agent agent = team.agents[j];
                if (agent.action.type.empty()) continue;
                if (conflict[agent.action.targetCoord] > 1) continue;
                int x, y, tox, toy;
                string type;
                x = agent.coord.x;
                y = agent.coord.y;
                tox = agent.action.targetCoord.x;
                toy = agent.action.targetCoord.y;
                type = agent.action.type;
                if (type == MOVE) {
                    if (walls[toy][tox] == 0 || walls[toy][tox] == team.teamID) {
                        teams[i].agents[j].coord = agent.action.targetCoord;
                        teams[i].agents[j].action.type = "";
                        walls[toy][tox] = team.teamID;
                        f = true;
                    }
                }
                if (type == REMOVE) {
                    walls[toy][tox] = 0;
                    teams[i].agents[j].action.type = "";
                }
                if (type == SET) {
                    if (x < 0 && y < 0) {
                        if (walls[toy][tox] == 0 || walls[toy][tox] == team.teamID) {
                            teams[i].agents[j].coord = agent.action.targetCoord;
                            walls[toy][tox] = team.teamID;
                        }
                    }
                }
            }
        }
    }
}
void paintArea() {
    bool painted[MAX_SIDE][MAX_SIDE] = {};
    int d[3] = {1,0,-1};

    rep(i,h-2) rep(j,w-2) {
        if (walls[i][j] == walls[i][j+1] && walls[i][j] == walls[i+1][j] && !walls[i+1][j+1] && !painted[i+1][j+1]) {
            bool isArea = true;
            bool visited[MAX_SIDE][MAX_SIDE] = {};
            int teamID = walls[i][j];
            queue<Vec2> que, willPaint;
            que.push({j+1,i+1});
            while(que.size()) {
                Vec2 coord = que.front(); que.pop();
                int x = coord.x, y = coord.y;
                if (visited[y][x] || walls[y][x] == teamID) continue;
                if (y == 0 || x == 0 || y == h-1 || x == w-1) { isArea = false; break; }
                if (!walls[y][x]) willPaint.push(coord);
                visited[y][x] = true;
                rep(I,3) rep(J,3) {
                    int Y = y+d[I], X = x+d[J];
                    que.push({X,Y});
                }
            }
            if (isArea) {
                while(willPaint.size()) {
                    Vec2 coord = willPaint.front(); willPaint.pop();
                    areas[coord.y][coord.x] = teamID;
                }
            }
        }
    }
}
void calcScore() {
    rep(i,h) rep(j,w) rep(k,TEAM_NUM) {
        if (teams[k].teamID == walls[i][j]) teams[k].wallPoint += points[i][j];
        if (teams[k].teamID == areas[i][j]) teams[k].areaPoint += abs(points[i][j]);
    }
}

