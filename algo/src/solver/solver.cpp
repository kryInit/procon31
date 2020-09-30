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

int h, w, agentNum, turns, turn;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];

int main() {

}
