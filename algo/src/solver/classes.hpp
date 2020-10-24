#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <string>
#include "constant.hpp"

struct Vec2 {
    int x,y;
    bool operator<( const Vec2& another ) const {
        return x == another.x ? y < another.y : x < another.x;
    }
    bool operator==( const Vec2& another ) const {
        return x == another.x && y == another.y;
    }
    bool operator!=( const Vec2& another ) const {
        return x != another.x || y != another.y;
    }

    Vec2() { x = y = 0; }
    Vec2(int _x, int _y) { x = _x; y = _y; }
};
struct Action {
    Vec2 targetCoord;
    std::string type="";

    Action() { type = ""; targetCoord = Vec2(); }
    Action(std::string _type, Vec2 _targetCoord) { type = _type; targetCoord = _targetCoord; }
};
struct Agent {
    int agentID;
    Vec2 coord;
    Action action;

    Agent() { agentID = -1; coord = Vec2(-1, -1); action = Action(); }
    Agent(int _agentID, Vec2 _coord) { agentID = _agentID; coord = _coord; action = Action(); }
};
struct Team {
    int teamID, wallPoint, areaPoint;
    Agent agents[MAX_AGENT];
};


struct Path {
    int pointSum;
    std::vector<Action> path;
    bool operator<( const Path& another ) const {
        return pointSum < another.pointSum;
    }

    Path() { pointSum = 0; path = std::vector<Action>();}
};

#endif
