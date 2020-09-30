#include <iostream>
#include <fstream>
#include <string>
#include "game.h"

extern bool CAN_USE_BLUE_FORCED_ACTIONS;
extern bool CAN_USE_ORANGE_FORCED_ACTIONS;
extern bool CAN_SHOW_ACTION[TEAM_NUM];
extern bool CAN_USE_PS4_CONTROLLER;

using namespace std;

Vec2 coord2idx(Vec2 coord) {
    return Vec2((int)(coord.x-H_WINDOW_MARGIN-WINDOW_LR_MARGIN)/TILE_SIZE, (int)(coord.y-H_WINDOW_MARGIN)/TILE_SIZE);
}

Vec2 idx2coord(Vec2 idx) {
    const int margine = H_TILE_SIZE + H_WINDOW_MARGIN;
    return Vec2(idx.x*TILE_SIZE+margine + WINDOW_LR_MARGIN, idx.y*TILE_SIZE+margine);
}

Game::Game() {}

Game::Game(int _matchID) {
    for(int i = 0; i<MAX_SIDE; ++i) {
        for(int j = 0; j<MAX_SIDE; ++j) {
            int x = j*TILE_SIZE+H_WINDOW_MARGIN + WINDOW_LR_MARGIN;
            int y = i*TILE_SIZE+H_WINDOW_MARGIN;
            int center_x = x + H_TILE_SIZE;
            int center_y = y + H_TILE_SIZE;
            sTiles[i][j].set(x, y, TILE_SIZE, TILE_SIZE);
            sAgents[i][j].set(center_x, center_y, TILE_SIZE*0.45);
        }
    }
    border = 0;
    matchID = _matchID;
    ifstream ifs("../data/" + to_string(matchID) + "/gameInfo");
    int _;
    ifs >> _ >> myTeamID >> turns;
    field = Field(matchID, myTeamID);
    Scene::SetBackground(Palette::Black);
    Window::Resize(TILE_SIZE*field.getWidth()+WINDOW_MARGIN+2*WINDOW_LR_MARGIN, TILE_SIZE*field.getHeight()+WINDOW_MARGIN);
    Window::Centering();
    
    if (matchID) CAN_USE_ORANGE_FORCED_ACTIONS = false;
    else CAN_USE_ORANGE_FORCED_ACTIONS = true;
}

void Game::getActions() {
    
    if (CAN_USE_PS4_CONTROLLER) getActionsDS();
    
    // change border
    if (KeyUp.down() || (KeyUp.pressedDuration() > 0.15s)) border = min(border+1, 17);
    if (KeyDown.down() || (KeyDown.pressedDuration() > 0.15s)) border = max(border-1, -16);
    
    
    // 以下でAgentのActionを得ている
    static Vec2 startCoord;
    static string type;
    static Color actionColor;
    static bool whileDragging=false;
    Vec2 cpos = Cursor::Pos();
    Vec2 idx = coord2idx(cpos);
    
    // cancel
    if (KeyC.down()) {
        if (!KeyShift.pressed() && CAN_USE_BLUE_FORCED_ACTIONS) field.cancelAction(0, idx, algosActions);
        if (KeyShift.pressed() && CAN_USE_ORANGE_FORCED_ACTIONS) field.cancelAction(1, idx, algosActions);
    }
    
    // s押下時にteams[0]のAgentが、s+shift押下時にteams[1]のAgentがカーソルの座標にセットされる
    if (KeyS.down())  {
        if (!KeyShift.pressed() && CAN_USE_BLUE_FORCED_ACTIONS) field.assignSETAction(0, idx, algosActions);
        if (KeyShift.pressed() && CAN_USE_ORANGE_FORCED_ACTIONS) field.assignSETAction(1, idx, algosActions);
    }

    // AorDが押された際、そこにエージェントがいるかどうかと手動操作が可能かのフラグを見てwhileDraggingをONにする
    if (KeyA.down() || KeyD.down()) {
        if (KeyA.down()) { type = MOVE; actionColor = MOVE_ACTION_COLOR; }
        if (KeyD.down()) { type = REMOVE; actionColor = REMOVE_ACTION_COLOR; }
        
        startCoord = idx2coord(idx);
        
        int teamID = field.agentExists(idx);
        whileDragging = field.isSafeIdx(idx) &&
                        ((teamID == field.getTeam(0).getTeamID() && CAN_USE_BLUE_FORCED_ACTIONS) ||
                         (teamID == field.getTeam(1).getTeamID() && CAN_USE_ORANGE_FORCED_ACTIONS));
    }
    
    // キー降下中かつwhileDraggingがONであれば矢印を描画
    // ここで描画しているためGame.drow()する前にこの関数を実行すると後ろに隠れてしまう。
    if ((KeyA.pressed() || KeyD.pressed()) && whileDragging) {
        Line(startCoord, cpos).drawArrow(5, Vec2(10, 10), actionColor);
    }
    
    // whileDraggingがONでキーを話したときx,yそれぞれの軸の差が1以下であることを確認してアクションとして登録している
    if ((KeyA.up() || KeyD.up()) && whileDragging) {
        Vec2 startIdx = coord2idx(startCoord);
        Vec2 endIdx = coord2idx(cpos);
        
        if (field.isSafeIdx(endIdx) && abs(startIdx.x - endIdx.x) <= 1 && abs(startIdx.y - endIdx.y) <= 1) {
            int teamID = field.agentExists(startIdx);
            int teamIdx = (teamID == field.getTeam(0).getTeamID() ? 0 : 1);
            int agentIdx = field.getTeam(teamIdx).getAgentsIdxFromCoord(startIdx);
            
            if (0 <= agentIdx) {
                Action action(type, endIdx);
                field.setAgentsAction(teamIdx, agentIdx, action);
            }
        }
        
        whileDragging = false;
    }
}

void Game::transition() {
    if (matchID == 0) {
        if (field.getTurn() < turns) return;
        system("simpleServer transition");
        field.updateFieldInfo(matchID);
    }
}

void Game::updateInfo() {
    ifstream ifs("../data/" + to_string(matchID) + "/action");
    algosActions.clear();
    int n;
    ifs >> n;
    for(int i=0; i<n; ++i) {
        int agentID, x, y;
        string type;
        ifs >> agentID >> x >> y >> type;
        algosActions[agentID] = Action((ALGO + type), Vec2(x,y));
    }
    field.updateFieldInfo(matchID);
}

void Game::dumpActions() {
    ofstream ofs("../data/" + to_string(matchID) + "/forcedAction");
    vector<Agent> agents = field.getAllTeamsAgentsWithActiveAction();
    ofs << agents.size() << endl;
    for(auto agent : agents) {
        Action action = agent.getAction();
        ofs << agent.getAgentID() << " " << action.getTargetCoord().x << " " << action.getTargetCoord().y << " " << action.getType() << endl;
    }
}

void Game::draw() {
    // tileの描画
    for(int i=0; i<field.getHeight(); ++i) {
        for(int j=0; j<field.getWidth(); ++j) {
            Vec2 coord(j,i);
            Color tileColor = field.getTileColor(coord);
            if (tileColor != Palette::White || field.getPoints(coord) >= border) sTiles[i][j].draw(tileColor);
            sTiles[i][j].drawFrame(1,0,Palette::Black);
        }
    }
    
    // 配置済Agentの描画
    for(int i=0; i<TEAM_NUM; ++i) {
        Team team = field.getTeam(i);
        for(int j=0; j<team.getAgentNum(); ++j) {
            Vec2 coord = team.getAgent(j).getCoord();
            int x = coord.x, y = coord.y;
            if (field.isSafeIdx(coord)) {
                sAgents[y][x].draw(team.getAgentColor());
                sAgents[y][x].drawFrame(1,0,Palette::Black);
            }
        }
    }
    
//    枠線の描画(drawFrameがH*W回実行されるのに対して一気にやれば(H+W)回だからこっちの方が早いと思うけど今のところ必要ないかも)
//    int hmargin = H_WINDOW_MARGIN, wmargin = WINDOW_LR_MARGIN+H_WINDOW_MARGIN;
//    int h = field.getHeight(), w = field.getWidth();
//    for(int i=0; i<=h; ++i) Line(wmargin, hmargin+i*TILE_SIZE, wmargin+w*TILE_SIZE, hmargin+i*TILE_SIZE).draw(2,Palette::Black);
//    for(int i=0; i<=w; ++i) Line(wmargin+i*TILE_SIZE, hmargin, wmargin+i*TILE_SIZE, hmargin+h*TILE_SIZE).draw(2,Palette::Black);
    
    
    // set action と 未配置エージェントの描画用にsettableAgentCoordとsenterを作成
    // TEAM_NUM == 2が前提となっている
    // todo 両チームの動き方に差分をつける
    const int csa[2] = {field.getTeam(0).countSettableAgent(), field.getTeam(1).countSettableAgent()};
    Vec2 settableAgentCoord[2][MAX_AGENT];
    const Vec2 center[2] = { Vec2(WINDOW_LR_MARGIN + field.getWidth()*TILE_SIZE+H_WINDOW_MARGIN*2 + WINDOW_LR_MARGIN/2,
                             H_WINDOW_MARGIN*2 + (field.getHeight()-4)*TILE_SIZE),
                             Vec2(WINDOW_LR_MARGIN/2, 4*TILE_SIZE) };
    {

        // 円座標系における角度座標
        const double t = Scene::Time();
        const double ta[4] = {t/6 - (int)(t/6),t/10 - (int)(t/10), t/15 - (int)(t/15), t/60 - (int)(t/60)};
        const double r = 10;
        
        for (int i=0; i<2; ++i) {
            Team team = field.getTeam(i);
            for (auto j : step(csa[i])) {
                const double theta = 360_deg/csa[i]*j + 360_deg*(EaseInOutQuad(ta[i]) +
                                                            EaseInOutCubic(ta[i+1]) +
                                                            EaseInOutExpo(ta[i+2]) +
                                                            EaseInOutSine(ta[(i+3)%4])
                                                            );
                
                settableAgentCoord[i][j] = OffsetCircular(center[i], max(50., csa[i]*r), theta);
            }
        }
    }
    
    // actionの描画
    {
        int setActionCount[2] = {};
        // forcedActionの描画
        for(int i=0; i<TEAM_NUM; ++i) {
            vector<Agent> agents = field.getTeam(i).getAgentsWithActiveAction();
            for(auto agent : agents) {
                Action agentsAction = agent.getAction();
                Vec2 startCoord = idx2coord(agent.getCoord());
                Vec2 endCoord = idx2coord(agentsAction.getTargetCoord());
                if (agentsAction.getType() == SET) {
                    startCoord = settableAgentCoord[i][setActionCount[i]];
                    setActionCount[i] = (setActionCount[i]+1 >= csa[i] ? 0 : setActionCount[i]+1);
                }
                Line(startCoord, endCoord).drawArrow(5, Vec2(10, 10), agentsAction.getActionColor());
            }
        }
        // actionの描画
        for(int i=0; i<TEAM_NUM; ++i) {
            Team team = field.getTeam(i);
            for(int j=0; j<team.getAgentNum(); ++j) {
                Agent agent = team.getAgent(j);
                if (agent.getAction().getType().empty() && algosActions.count(agent.getAgentID())) {
                    Action agentsAction = algosActions[agent.getAgentID()];
                    Vec2 startCoord = idx2coord(agent.getCoord());
                    Vec2 endCoord = idx2coord(agentsAction.getTargetCoord());
                    if (agentsAction.getType() == ALGOSET) {
                        startCoord = settableAgentCoord[i][setActionCount[i]];
                        setActionCount[i] = (setActionCount[i]+1 >= csa[i] ? 0 : setActionCount[i]+1);
                    }
                    Line(startCoord, endCoord).drawArrow(5, Vec2(10, 10), agentsAction.getActionColor());
                }
            }
        }
    }

    
    // tilePointの描画
    for(int i=0; i<field.getHeight(); ++i) for(int j=0; j<field.getWidth(); ++j) {
        Vec2 coord(j,i);
        Color color = (field.getTileColor(coord) == Palette::White && field.getPoints(coord) < border ? Palette::White : Palette::Black);
        FontAsset(U"tilePoint")(field.getPoints(coord)).draw(Arg::center = Vec2(j*TILE_SIZE+TILE_SIZE+WINDOW_LR_MARGIN-11, i*TILE_SIZE+TILE_SIZE-11), color);
    }
    
    // 未配置エージェントの描画(TEAM_NUM=2の前提で書いている)
    {
        for (int i=0; i<2; ++i) {
            Team team = field.getTeam(i);
            int n = team.countSettableAgent();
            for (auto j : step(n)) Circle(settableAgentCoord[i][j], 19).draw(team.getAgentColor());
            FontAsset(U"agentNum")(n).draw(Arg::center = center[i], Palette::White);
        }
    }
    
    // 点数の描画(TEAM_NUM=2の前提で書いている)
    {
        const int wm = 100;
        const int minhm = TILE_SIZE*1.5, maxhm = TILE_SIZE*3.5;
        const Vec2 drawPos[2][3] = {
            { Vec2(WINDOW_LR_MARGIN+field.getWidth()*TILE_SIZE+WINDOW_MARGIN+wm, minhm),
              Vec2(WINDOW_LR_MARGIN+field.getWidth()*TILE_SIZE+WINDOW_MARGIN+wm, maxhm),
              Vec2(WINDOW_LR_MARGIN+field.getWidth()*TILE_SIZE+WINDOW_MARGIN+wm+200, (minhm+maxhm)/2)
            },
            { Vec2(H_WINDOW_MARGIN+wm, H_WINDOW_MARGIN+field.getHeight()*TILE_SIZE-maxhm),
              Vec2(H_WINDOW_MARGIN+wm, H_WINDOW_MARGIN+field.getHeight()*TILE_SIZE-minhm),
              Vec2(H_WINDOW_MARGIN+wm+200, H_WINDOW_MARGIN+field.getHeight()*TILE_SIZE-((minhm+maxhm)/2))
            }
        };
        for (int i=0; i<2; ++i) {
            Team team = field.getTeam(i);
            const int p[3] = {team.getWallPoint(), team.getAreaPoint(), team.getWallPoint()+team.getAreaPoint()};
            const Color c[3] = {team.getWallTileColor(), team.getAreaTileColor(), team.getAgentColor()};
            
            for(int j=0; j<3; ++j) {
                Rect tmp = FontAsset(U"score")(p[j]).region(Arg::rightCenter = drawPos[i][j]);
                tmp.w += 30;
                tmp.x -= 15;
                tmp.h += 10;
                tmp.y -= 4;
                tmp.rounded(10).drawFrame(4,0,c[j]);
                FontAsset(U"score")(p[j]).draw(Arg::rightCenter = drawPos[i][j], Palette::White);
                
            }
        }
    }
    
    // residual turn の描画
    FontAsset(U"littleInfo")(U"{}/{}"_fmt(field.getTurn(), turns)).draw(Arg::topLeft = Vec2(8,4), Palette::White);
    
    //matchIDの描画
    FontAsset(U"littleInfo")(matchID).draw(Arg::bottomRight = Vec2(WINDOW_LR_MARGIN*2+WINDOW_MARGIN+field.getWidth()*TILE_SIZE-8, WINDOW_MARGIN+field.getHeight()*TILE_SIZE-4), Palette::White);
    
    if (CAN_USE_PS4_CONTROLLER) drawDS();
}

void Game::getActionsDS() {
    const auto gamepad = Gamepad(0);
    
    // change border
    if (Gamepad(0).buttons[6].down() || Gamepad(0).buttons[6].pressedDuration() > 0.15s) border = min(border+1, 17);
    if (Gamepad(0).buttons[7].down() || Gamepad(0).buttons[7].pressedDuration() > 0.15s) border = max(border-1, -16);

    // change Ps4cursor position
    double dt = Scene::DeltaTime();
    double dx = (int)(gamepad.axes[0]*10)*100*dt + (int)(gamepad.axes[2]*10)*20*dt;
    double dy = (int)(gamepad.axes[1]*10)*100*dt + (int)(gamepad.axes[5]*10)*20*dt;
    PS4Cursor.x += dx;
    PS4Cursor.y += dy;
    PS4Cursor.x = min((double)(WINDOW_LR_MARGIN*2+WINDOW_MARGIN+TILE_SIZE*field.getWidth()), max(0., PS4Cursor.x));
    PS4Cursor.y = min((double)(WINDOW_MARGIN+TILE_SIZE*field.getHeight()), max(0., PS4Cursor.y));

    
    // get agent's action to the last of the function
    // gamepad.buttons[5](R1)押下時であれば時計回りに45度回転した向きで、押下時でなければそのままの向きで
    Vec2 idx = coord2idx(PS4Cursor);
    string type = (gamepad.buttons[4].pressed() ? REMOVE : MOVE);
    if (gamepad.buttons[5].pressed()) {
        Vec2 didx[4] = {Vec2(-1,-1), Vec2(-1,1), Vec2(1,1), Vec2(1,-1)};
        for(int i=0; i<4; ++i) if (gamepad.buttons[i].down()) field.setAction2CloseAgent(idx, didx[i], type);
    } else {
        Vec2 didx[4] = {Vec2(-1,0), Vec2(0,1), Vec2(1,0), Vec2(0,-1)};
        for(int i=0; i<4; ++i) if (gamepad.buttons[i].down()) field.setAction2CloseAgent(idx, didx[i], type);
    }
    
    //get set action
    // buttons[15](↓)でteamIdx==0のチームのセットbuttons[16](↓)でteamIdx==1のチームのキャンセル
    // buttons[17](←)でteamIdx==0のチームのセットbuttons[14](↑)でteamIdx==1のチームのキャンセル
    if (gamepad.buttons[15].down()) field.assignSETAction(0, idx, algosActions);
    if (gamepad.buttons[16].down()) field.cancelAction(0, idx, algosActions);
    if (gamepad.buttons[17].down()) field.assignSETAction(1, idx, algosActions);
    if (gamepad.buttons[14].down()) field.cancelAction(1, idx, algosActions);
}

void Game::drawDS() {
    Circle(PS4Cursor, 10).draw(Palette::Pink);
    Circle(PS4Cursor, 10).drawFrame(1, Palette::Black);
}
