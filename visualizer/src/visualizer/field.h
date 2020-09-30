#ifndef field_h
#define field_h

#include "constant.h"
#include "team.h"


class Field {
    int height, width, turn;
    int walls[MAX_SIDE][MAX_SIDE],  areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
    Team teams[TEAM_NUM];
    
public:
    Field();
    Field(int matchID, int myTeamID);
    void updateFieldInfo(int matchID);
    

    int getHeight();
    int getWidth();
    int getTurn();
//    int getWalls(Vec2 coord);
//    int getAreas(Vec2 coord);
    int getPoints(Vec2 coord);
    Team getTeam(int idx);
    Color getTileColor(Vec2 coord);
    
    int agentExists(Vec2 coord);
    
    void setAgentsAction(int teamIdx, int agentIdx, Action action);
    void setAction2CloseAgent(Vec2 coord, Vec2 dcoord, std::string type);
    void setActionWithLine(Vec2 s, Action action);
    void assignSETAction(int teamIdx, Vec2 coord, std::map<int, Action> algosAction);
    void cancelAction(int teamIdx, Vec2 coord, std::map<int, Action> algosAction);
    
    std::vector<Agent> getAllTeamsAgentsWithActiveAction();
    
    bool isSafeIdx(Vec2 idx);
};

inline int Field::getHeight() { return height; }
inline int Field::getWidth() { return width; }
inline int Field::getTurn() { return turn; }
//inline int Field::getWalls(Vec2 coord) { return walls[(int)coord.y][(int)coord.x]; }
//inline int Field::getAreas(Vec2 coord) { return areas[(int)coord.y][(int)coord.x]; }
inline int Field::getPoints(Vec2 coord) { return points[(int)coord.y][(int)coord.x]; }
inline Team Field::getTeam(int idx) { return teams[idx]; }
inline Color Field::getTileColor(Vec2 coord) {
    int x = coord.x, y = coord.y;
    for(int i=0; i<TEAM_NUM; ++i) {
        int teamID = teams[i].getTeamID();
        if (teamID == walls[y][x]) return teams[i].getWallTileColor();
        if (teamID == areas[y][x]) return teams[i].getAreaTileColor();
    }
    return Palette::White;
}

inline void Field::setAgentsAction(int teamIdx, int agentIdx, Action action) {
    teams[teamIdx].setAgentsAction(agentIdx, action);
}

inline int Field::agentExists(Vec2 coord) {
    for(int i=0; i<TEAM_NUM; ++i) if (teams[i].agentExists(coord)) return teams[i].getTeamID();
    return 0;
}

inline bool Field::isSafeIdx(Vec2 idx) {
    return 0 <= idx.x && idx.x < width && 0 <= idx.y && idx.y < height;
}

#endif
