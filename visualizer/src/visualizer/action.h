#ifndef action_h
#define action_h

class Action {
    std::string type;
    Vec2 targetCoord;
    
public:
    Action() { type = ""; targetCoord = Vec2(); }
    Action(std::string _type, Vec2 _targetCoord) { type = _type; targetCoord = _targetCoord; }
    
    bool operator==(const Action& action) const { return type == action.type && targetCoord == action.targetCoord; }
    
    bool isActiveAction();
    
    std::string getType();
    Vec2 getTargetCoord();
    Color getActionColor();
};

inline bool Action::isActiveAction() { return !type.empty(); }

inline std::string Action::getType() { return type; }
inline Vec2 Action::getTargetCoord() { return targetCoord; }

inline Color Action::getActionColor() {
    if (type == MOVE) return MOVE_ACTION_COLOR;
    if (type == REMOVE) return REMOVE_ACTION_COLOR;
    if (type == SET) return SET_ACTION_COLOR;
    if (type == ALGOMOVE) return ALGOMOVE_ACTION_COLOR;
    if (type == ALGOREMOVE) return ALGOREMOVE_ACTION_COLOR;
    if (type == ALGOSET) return ALGOSET_ACTION_COLOR;
    return Color(0,0,0);
}

#endif
