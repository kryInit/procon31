#ifndef constant_h
#define constant_h

const int MAX_SIDE = 24;
const int MIN_SIDE = 12;
const int MAX_AGENT = 14;
const int MIN_AGENT = 6;
const int TEAM_NUM = 2;

const int TILE_SIZE = 42;
const int H_TILE_SIZE = TILE_SIZE/2;
const int WINDOW_MARGIN = 20;
const int H_WINDOW_MARGIN = WINDOW_MARGIN/2;
const int WINDOW_LR_MARGIN = 400;
const std::string MOVE = "move";
const std::string REMOVE = "remove";
const std::string SET = "set";
const std::string STAY = "stay";
const std::string ALGO = "algos_";
const std::string ALGOMOVE = "algos_move";
const std::string ALGOREMOVE = "algos_remove";
const std::string ALGOSET = "algos_set";
const std::string ALGOSTAY = "algos_stay";

const Color MOVE_ACTION_COLOR = Color(0,102,204);
const Color REMOVE_ACTION_COLOR = Color(255,102,51);
const Color SET_ACTION_COLOR = Color(100,100,100);
const Color ALGOMOVE_ACTION_COLOR = Color(0,60,120);
const Color ALGOREMOVE_ACTION_COLOR = Color(150,60,30);
const Color ALGOSET_ACTION_COLOR = Color(60,60,60);

#endif
