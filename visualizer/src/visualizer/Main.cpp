# include <Siv3D.hpp> // OpenSiv3D v0.4.3
# include <sys/stat.h>
# include <iostream>
# include <fstream>
# include <string>
# include "game.h"

using namespace std;

bool CAN_USE_BLUE_FORCED_ACTIONS=true;
bool CAN_USE_ORANGE_FORCED_ACTIONS=true;
bool CAN_SHOW_ACTION[TEAM_NUM] = {true, false};
bool CAN_USE_PS4_CONTROLLER;

void runGame(Game &game);
void registMatch(TextEditState &tes, Transition &press);


vector<int> registeredMatchID;
int nowMatchIdx = 0;

void Main()
{
    // todo 好きなコントローラーを選べるようにする。
    CAN_USE_PS4_CONTROLLER = (bool)(Gamepad(0));

    Console.open();
    
    // 自分がクレーを使いたいから条件分岐をしている
    if (struct stat st; stat("./Klee.ttc", &st)) {
        FontAsset::Register(U"littleInfo", 22);
        FontAsset::Register(U"score", 30);
        FontAsset::Register(U"agentNum", 30);
        FontAsset::Register(U"tilePoint", 20);
        FontAsset::Register(U"massage", 30);
        FontAsset::Register(U"regist", 20);
    } else {
        FontAsset::Register(U"littleInfo", 22, U"./Klee.ttc");
        FontAsset::Register(U"score", 30, U"./Klee.ttc");
        FontAsset::Register(U"agentNum", 30, U"./Klee.ttc");
        FontAsset::Register(U"tilePoint", 22, U"./Klee.ttc");
        FontAsset::Register(U"massage", 30, U"./Klee.ttc");
        FontAsset::Register(U"regist", 20, U"./Klee.ttc");
    }

    
    Game game;

    TextEditState tes;
    Transition press(0.05s, 0.05s);
    
    bool registeringMatchID = true;
    
    Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
    Window::Resize(800, 600);
    
    while (System::Update())
    {
        ClearPrint();
        
        // rを押した時にregistration of matchIDに移行(matchID==0の時に行えるゲームのre-generate(command+r)と区別
        if (!registeringMatchID && !KeyCommand.pressed() && KeyR.down()) {
            Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
            Window::Resize(800, 600);
            registeringMatchID = true;
            // registrationに切り替えた際にrが入力されてしまうためcontinueしている
            continue;
        }
        
        // registered matchIDが一つ以上ある状態でtabを押すことでvisualizeするmatchを切り替えることができる
        if ((KeyTab.down() || (CAN_USE_PS4_CONTROLLER && Gamepad(0).buttons[9].down())) && registeredMatchID.size()) {
            nowMatchIdx = (registeringMatchID ? 0 : (nowMatchIdx+1)%registeredMatchID.size());
            for(int _=0; _<(int)(registeredMatchID.size()); ++_) {
                if (struct stat st; stat(("../data/" + to_string(registeredMatchID[nowMatchIdx]) + "/fieldInfo").c_str(), &st) == 0) {
                    game = Game(registeredMatchID[nowMatchIdx]);
                    registeringMatchID = false;
                    break;
                }
                nowMatchIdx = (nowMatchIdx+1)%((int)(registeredMatchID.size()));
            }
        }
        
        if (registeringMatchID) registMatch(tes, press);
        else runGame(game);
    }
}
 

void registMatch(TextEditState &tes, Transition &press) {
    const RoundRect registRRect = Rect(670, 70, 100, 30).rounded(10);
    static set<int> matchIDs;
    static String result;
    static int id;
    
    static bool existingMatchID = false;
    
    // title(?)
    FontAsset(U"massage")(U"matchID registration").draw(Arg::topLeft = Vec2(20, 15), Palette::Black);
    
    // textboxから文字列を受け取り、matchIDかerror massageを取得,描画
    if (SimpleGUI::TextBox(tes, Vec2(40, 70), 600))
    {
        existingMatchID = false;
        result = tes.text;
        try
            {
                id = Parse<int>(result);
                result = Format(id);
                if (id < 0) result = U"The matchID must be a positive number";
                else if (struct stat st; stat(("../data/" + to_string(id)).c_str(), &st)) result = U"There is no match with the matchID: " + result;
                else if (matchIDs.count(id)) result = U"Already registered";
                else {
                    result = U"id: " + result;
                    existingMatchID = true;
                }
            }
            catch (const ParseError& e)
            {
                if (tes.text.empty()) result = U"";
                else result = U"parse error";
            }
    }
    FontAsset(U"massage")(result).draw(60, 120, ColorF(0.25));
    
    // regist buttonの描画
    press.update(registRRect.leftPressed() || (KeyCommand.pressed() && KeyR.pressed()));
    const double t = press.value();
    registRRect.movedBy(Vec2(0, 0).lerp(Vec2(0, 4), t)).drawShadow(Vec2(2, 3).lerp(Vec2(0.3, 0.5), t), 7 - t * 7, 4 - t * 4).draw(Palette::White);
    registRRect.movedBy(Vec2(0, 0).lerp(Vec2(0, 4), t)).drawFrame(3,0,Palette::Black);
    FontAsset(U"regist")(U"regist").draw(Arg::center = Vec2(720, 83+4*t), Palette::Black);
    
    // regist matchID
    if(existingMatchID && t == 1) {
        result = U"";
        tes.text.clear();
        matchIDs.insert(id);
    }
    
    // Registered matchIDの描画
    if (matchIDs.size()) FontAsset(U"regist")(U"List of registerd matchIDs").draw(40, 190, Palette::Black);
    {
        registeredMatchID.clear();
        const int h = 5, w = 2;
        const double minh = 250, maxh = 500, minw = 250, maxw = 650;
        const double dh = (maxh-minh)/(h-1), dw = (maxw-minw)/(w-1);
        int th = 0;
        for(int matchID : matchIDs) {
            if (th < 10) {
                FontAsset(U"massage")(matchID).draw(Arg::rightCenter = Vec2(minw + dw*(th/5), minh + dh*(th%5)), Palette::Black);
                th++;
            }
            registeredMatchID.push_back(matchID);
        }
        if (matchIDs.size() > 10) {
            FontAsset(U"massage")(U"etc...").draw(Arg::bottomRight = Vec2(750, 570), Palette::Black);
        }
    }
}

void runGame(Game &game) {
    static double deltaTime=0;
    deltaTime += Scene::DeltaTime();
    game.draw();
    if (game.isUnfinished()) game.getActions();
    
    if (((KeyCommand + KeyR).down() || (CAN_USE_PS4_CONTROLLER && Gamepad(0).buttons[12].down())) && game.getMatchID() == 0) {
        system("./simpleServer gen");
        game = Game(0);
    }
    
    if ((KeyEnter.down() || (CAN_USE_PS4_CONTROLLER && Gamepad(0).buttons[13].down())) && game.getMatchID() == 0) {
        if (game.isUnfinished()) system("./simpleServer transition");
    }
    
    if (deltaTime > 0.1 && game.isUnfinished()) {
        game.updateInfo();
        game.dumpActions();
        
        deltaTime = 0;
    }
}

