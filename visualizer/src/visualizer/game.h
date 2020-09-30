#ifndef game_h
#define game_h

#include <Siv3D.hpp>
#include "constant.h"
#include "field.h"

class Game {
    Rect sTiles[MAX_SIDE][MAX_SIDE];
    Circle sAgents[MAX_SIDE][MAX_SIDE];
    
    Field field;
    std::map<int, Action> algosActions;
    
    int matchID, myTeamID, turns, border;
    
    Vec2 PS4Cursor = Vec2(10,10);
    
    void getActionsDS();
    void drawDS();

public:
    Game();
    Game(int _matchID);

    int getMatchID();
    void getActions();

    void transition();
    void updateInfo();
    void dumpActions();
    void draw();
    
    bool isUnfinished();
};

inline int Game::getMatchID() { return matchID; }
inline bool Game::isUnfinished() { return field.getTurn() < turns; }

#endif
