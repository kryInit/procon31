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
#ifdef DEBUG
    cout << "solver!" << endl;
#endif
    output();
}
