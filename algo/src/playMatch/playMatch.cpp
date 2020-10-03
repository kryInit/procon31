#include<bits/stdc++.h>
#include<unistd.h>
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 5) return 0;

    const string pjtPath = (string)(((filesystem::path)(string(argv[0]))).parent_path());
    const string token = argv[1];
    const string URL = argv[2];
    const string teamID = argv[3];
    const string matchID = argv[4];

    const string op = token + " " + URL + " " + teamID + " " + matchID;
    const string utilityPath = pjtPath + "/utility";
    const string dataPath = pjtPath + "/data/" + matchID;

    system(("python " + utilityPath + "/makeGameInfo.py " + op).c_str());

    int nowTurn;
    int turns;

    {
        ifstream ifs(dataPath + "/gameInfo");
        int _;
        ifs >> _ >> _ >> turns;
    }
    {
        system(("python " + utilityPath + "/makeFieldInfo.py " + op).c_str());
        ifstream ifs(dataPath + "/fieldInfo");
        int _;
        ifs >> _ >> _ >> nowTurn;
    }

    if (nowTurn >= turns) return 0;

    nowTurn--;
    while(nowTurn < turns) {
        int tmpTurn = nowTurn;
        while(true) {
            system(("python " + utilityPath + "/makeFieldInfo.py " + op).c_str());
            ifstream ifs(dataPath + "/fieldInfo");
            int _;
            ifs >> _ >> _ >> nowTurn;
            if (tmpTurn != nowTurn) break;

            usleep(300000);
        }

        system((pjtPath + "/algo/bin/solver " + matchID + " &").c_str());
        system(("python " + utilityPath + "/comms.py " + op).c_str());
    }
}
