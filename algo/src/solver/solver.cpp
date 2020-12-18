#include<bits/stdc++.h>
#include "constant.hpp"
#include "classes.hpp"
using namespace std;

#define rep(i,n) for(int i=0;i<(int)(n);++i)
#define rep1(i,n) for(int i=1;i<=(int)(n);++i)
#define rep11(i,n) for(int i=1;i<(int)(n);++i)
#define repo(i,o,n) for(int i=o;i<(int)(n);++i)
#define repm(i,n) for(int i=(int)(n)-1;i>=0;--i)

unsigned Xseed[4];
void initXS(unsigned s) {
    for(int i=1; i<=4; i++){
        Xseed[i-1] = s = 1812433253 * (s^(s>>30)) + (unsigned)i;
    }
}
unsigned xs(void) {
    unsigned t;
    t=(Xseed[0]^(Xseed[0]<<11));
    Xseed[0]=Xseed[1];
    Xseed[1]=Xseed[2];
    Xseed[2]=Xseed[3];
    return Xseed[3]=(Xseed[3]^(Xseed[3]>>19))^(t^(t>>8));
}

void input();
void output();
void initialize(int argc, char *argv[]);
void finalize();

bool is_safe_coord(int x, int y);
bool is_safe_coord(Vec2 coord);

int h, w, agentNum, turns, turn, operationMillis;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];

int dist[MAX_SIDE][MAX_SIDE][MAX_SIDE][MAX_SIDE];
int board_state[MAX_SIDE][MAX_SIDE];

bool conflict_potential[MAX_SIDE][MAX_SIDE];
bool agent_exists[MAX_SIDE][MAX_SIDE];

const int d[3] = {-1,0,1};

int matchID;
string dataPath;
chrono::system_clock::time_point start;


// hyper parameter
//const int MAX_DEPTH = 5;
int MAX_DEPTH = 4;
const double SIMILARITY_COEF = 0.1, POINT_COEF = 1./16.;
const double TEMP_RADIX = 300;
const double PROBABILITY_COEF = 20;
const int MAX_PROC_SIZE_EACH_AGENT = 700;

void set_hyper_parameter() {
    MAX_DEPTH = 4 + (h*w - 12*12) / 72;
}

struct LAction {
    bool is_move;
    Vec2 target_coord;

    LAction() { is_move=false; target_coord=Vec2(-1,-1); }
    LAction(bool _is_move, Vec2 _target_toord) { is_move = _is_move; target_coord = _target_toord; }
};

int point_with_action_and_state(const LAction &action, int state) {
    const int y = action.target_coord.y, x = action.target_coord.x;
    const int point = points[y][x];
    if (state == 0) { // none
        if (action.is_move) return point;
        else return 0;
    } else if (state == 1) { // my wall
        if (action.is_move) return 0;
        else return -point;
    } else if (state == 2) { // op wall
        if (action.is_move) return 0;
        else return point;
    } else if (state == 3) { // my area
        if (action.is_move) return point - abs(point);
        else return 0;
    } else if (state == 4) { // op area
        if (action.is_move) return point + abs(point);
        else return 0;
    }
}

struct Procedure {
    vector<pair<Vec2, LAction>> proc;
    int p_sum=0;
    Vec2 start_coord;

    void update_p_sum(LAction action) {
        int state = now_state(action.target_coord);
        p_sum += point_with_action_and_state(action, state);
    }

    Procedure() { p_sum = 0; }
    Procedure(Vec2 _start_coord) {
        p_sum = 0;
        start_coord = _start_coord;
    }
    double eval() { return p_sum; }
    void push_action(LAction action) {
        update_p_sum(action);
        if (proc.empty()) proc.emplace_back(start_coord, action);
        else proc.emplace_back(proc.back().second.target_coord, action);
    }
    int proc_size() { return proc.size(); }
    Vec2 now_coord() {
        if (proc.empty()) return start_coord;
        else {
            if (proc.back().second.is_move) return proc.back().second.target_coord;
            else return proc.back().first;
        }
    }
    Vec2 latest_target_coord() {
        if (proc.empty()) return start_coord;
        else return proc.back().second.target_coord;
    }
    LAction latest_action() {
        if (proc.empty()) return LAction(true, start_coord);
        else return proc.back().second;
    }
    LAction first_action() {
        if (proc.empty()) return LAction(true, start_coord);
        else return proc.front().second;
    }
    int now_state(Vec2 target) {
        int state = board_state[target.y][target.x];
        for(const auto &i : proc) if (i.second.target_coord == target) {
            if (i.second.is_move) state = (state == 2 ? 2 : 1);
            else state = (state <= 2 ? 0 : state);
        }
        return state;
    }
    double similarity(int cnt[MAX_DEPTH][2][MAX_SIDE][MAX_SIDE]) {
//        int idx = 0;
//        double sum = 1.0 / cnt[idx++][start_coord.y][start_coord.x];
        int idx = 1;
        double sum = 0;
        for(const auto &i : proc) {
            if (cnt[idx][i.second.is_move][i.second.target_coord.y][i.second.target_coord.x] == 0) {
                cout << "ha?" << endl;
            }
            sum += 1.0 / cnt[idx++][i.second.is_move][i.second.target_coord.y][i.second.target_coord.x];
        }
        return sum;
    }
    void display_proc() {
        char c[MAX_SIDE][MAX_SIDE] = {};
        rep(i,h) rep(j,w) c[i][j] = ' ';
        cout << "start coord: (" << start_coord.x << ", " << start_coord.y << ")" << endl;
        c[start_coord.y][start_coord.x] = 'A';
        int cnt=0;
        for(auto i : proc) {
//            if (i.second.is_move) cout << "move: ";
//            else cout << "remove: ";
//            cout << "(" << i.first.x << ", " << i.first.y << "), (" << i.second.target_coord.x << ", " << i.second.target_coord.y << ")" << endl;
            int y = i.second.target_coord.y, x = i.second.target_coord.x;
            c[y][x] = ++cnt+'a';
        }
        cout << "------------------------------------------------" << endl;
        rep(i,h) {
            rep(j,w) cout << c[i][j] << " ";
            cout << endl;
        }
        cout << "------------------------------------------------" << endl;
    }
    vector<LAction> get_actions() {
        vector<LAction> actions;
        for(auto i : proc) actions.push_back(i.second);
        return actions;
    }
};


class ProcedureEnumerator {
    int target_cnt[MAX_DEPTH+1][2][MAX_SIDE][MAX_SIDE] = {};
    priority_queue<pair<double,int>> enumed_procs[MAX_DEPTH+1];
    vector<Procedure> storage[MAX_DEPTH+1];
    double ms_time_limit;
    chrono::system_clock::time_point sc_start, sc_now;

    bool is_within_time_limit() {
        sc_now = chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(sc_now - sc_start).count();
        return elapsed < ms_time_limit;
    }
    void push_all_transitioned_proc(Procedure proc) {
        Vec2 now_coord = proc.now_coord();
        rep(i,3) rep(j,3) if (i != 1 || j != 1) {
            int y = now_coord.y+d[i], x = now_coord.x+d[j];
//            cout << "(" << now_coord.x << ", " << now_coord.y << ") -> (" << x << ", " << y << ")" << endl;
            Vec2 target(x,y);
            if (is_safe_coord(target)) {
                LAction action;
                int state = proc.now_state(target);
                if (state == 1) {
                    action = LAction(true, target);
                    push_proc(proc, action);

                    action = LAction(false, target);
                } else if (state == 2) {
                    action = LAction(false, target);
                } else {
                    action = LAction(true, target);
                }
                push_proc(proc, action);
            }
        }
    }
    void push_proc(Procedure proc, LAction action) {
        proc.push_action(action);
        int idx = proc.proc_size();
        target_cnt[idx][action.is_move][action.target_coord.y][action.target_coord.x]++;
        storage[idx].push_back(proc);
        enumed_procs[idx].push(make_pair(eval_proc(proc), storage[idx].size()-1));
    }
    double eval_proc(Procedure proc) {
        const double a = (double)proc.similarity(target_cnt);
        const double e = proc.eval();
        return exp(e*POINT_COEF + a*SIMILARITY_COEF);
    }
    vector<Procedure> choice_procs(int depth) {
        vector<Procedure> proc;
        while(proc.size() < MAX_PROC_SIZE_EACH_AGENT && !enumed_procs[depth].empty()) {
            int idx = enumed_procs[depth].top().second;
            enumed_procs[depth].pop();
            proc.push_back(storage[depth][idx]);
        }
        return proc;
    }

public:
    ProcedureEnumerator() {
//        ms_time_limit = (operationMillis - 1000) / 2 / agentNum;
        ms_time_limit = (operationMillis - 1000) / 2;
        sc_start = chrono::system_clock::now();
    }
    vector<Procedure> enumerate_procedure(Vec2 start_coord, int depth) {
        depth = max(1, min(MAX_DEPTH, depth));
        storage[0].emplace_back(start_coord);
        enumed_procs[0].push(make_pair(0., 0));
        target_cnt[0][1][start_coord.y][start_coord.x]++;
        int now_depth = 0;
        int loop_count = 0;
        int empty_count=0;
        int border = 1e7 / MAX_DEPTH / 10;
        while(is_within_time_limit()) {
            if (loop_count == border) break;
            if (!enumed_procs[now_depth].empty()) {
                int idx = enumed_procs[now_depth].top().second;
                enumed_procs[now_depth].pop();
                push_all_transitioned_proc(storage[now_depth][idx]);
                empty_count = 0;
            } else empty_count++;
            now_depth = (now_depth+1 == depth ? 0 : now_depth+1);
            loop_count++;
            if (empty_count == depth) break;
        }
        cout << "loop_count: " << loop_count << " / " << border << endl;
        return choice_procs(depth);
    }
};

class NextActionDecider {
    double ms_time_limit;
    double now_max_eval=0;

    vector<Procedure> best_procs;
    chrono::system_clock::time_point sc_start, sc_now;

    bool is_within_time_limit() {
        sc_now = chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(sc_now - sc_start).count();
        return elapsed < ms_time_limit;
    }

    double tempreture() {
        sc_now = chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(sc_now - sc_start).count();
        return pow(TEMP_RADIX, elapsed / ms_time_limit);
    }
    double probability(double diff, double t) {
        if (diff > 0) return 1;
        else return exp(diff*t*PROBABILITY_COEF);
    }
    bool permit_transition(double diff) {
        return probability(diff, tempreture()) > ((double)xs() / (double)UINT_MAX);
    }

    double strict_eval(vector<Procedure> procs) {
        double point_sum = 0;
        int action_num = 0;
        vector<LAction> actions[MAX_AGENT];

        int targeted_count[MAX_SIDE][MAX_SIDE] = {};
        bool no_action[MAX_AGENT] = {};
        rep(i,agentNum) {
            Vec2 start_coord = procs[i].start_coord;
            actions[i] = procs[i].get_actions();
            if (!actions[i].empty()) {
                Vec2 target = actions[i].front().target_coord;
                int y = target.y,  x = target.x;
                targeted_count[y][x]++;
                if (!actions[i].front().is_move) targeted_count[start_coord.y][start_coord.x]++;
            }
        }
        rep(i,agentNum) {
            if (!actions[i].empty()) {
                Vec2 target = actions[i].front().target_coord;
                int y = target.y,  x = target.x;
                if (targeted_count[y][x] == 1 && conflict_potential[y][x] && !agent_exists[y][x]) {
                    rep(j,agentNum) {
                        int Y = teams[1].agents[j].coord.y, X = teams[1].agents[j].coord.x;
                        if (abs(Y-y) <= 1 && abs(X-x) <= 1) targeted_count[Y][X]++;
                    }
                }
            }
        }
        rep(i,agentNum) {
            if (!actions[i].empty()) {
                Vec2 target = actions[i].front().target_coord;
                int y = target.y,  x = target.x;
                if (targeted_count[y][x] == 1 && conflict_potential[y][x]) {
                    if (!agent_exists[y][x]) {
                        int e = procs[i].eval();
                        point_sum += e;
                    }
                    no_action[i] = true;
                    action_num += actions[i].size();
                }
            }
        }
        rep(i,agentNum) {
            if (!actions[i].empty()) {
                Vec2 target = actions[i].front().target_coord;
                int y = target.y,  x= target.x;
                if (targeted_count[y][x] >= 2) {
                    no_action[i] = true;
                    action_num += actions[i].size();
                }
            }
        }

        int states[MAX_SIDE][MAX_SIDE] = {};
        bool targeted[2][MAX_SIDE][MAX_SIDE] = {};
        rep(i,h) rep(j,w) states[i][j] = board_state[i][j];
        rep(i,MAX_DEPTH) rep(j,agentNum) if (!no_action[j]) {
            if (actions[j].size() <= i) continue;
            Vec2 target = actions[j][i].target_coord;
            bool is_move = actions[j][i].is_move;
            int y = target.y, x = target.x;
            if (!targeted[is_move][y][x]) {
                int point = 0;
                if (states[y][x] == 0) {
                    if (is_move) {
                        point = points[y][x];
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 1) {
                    if (!is_move) {
                        point = -points[y][x];
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 2) {
                    if (!is_move) {
                        point = points[y][x];
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 3) {
                    if (is_move) {
                        point = points[y][x] - abs(points[y][x]);
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 4) {
                    if (is_move) {
                        point = points[y][x] + abs(points[y][x]);
                        targeted[is_move][y][x] = true;
                    }
                }
                point_sum += point;
                action_num++;
            }
        }

        return point_sum / (double)max(1,action_num);
    }

    double eval(vector<Procedure> procs) {
        static double max_eval = 0;
        vector<LAction> actions[MAX_AGENT];
        rep(i,agentNum) actions[i] = procs[i].get_actions();

        int size = 0;
        bool no_action[MAX_AGENT] = {};
        rep(i,agentNum) {
            if (!actions[i].empty()) {
                size += actions[i].size();
                Vec2 target = actions[i].front().target_coord;
                no_action[i] = conflict_potential[target.y][target.x];
                rep(j,agentNum) {
                    if (i != j && !actions[j].empty()) {
                        Vec2 ot = actions[j].front().target_coord;
                        if (target == ot) no_action[i] = no_action[j] = true;
                    }
                }
            }
        }
        int states[MAX_SIDE][MAX_SIDE] = {};
        bool targeted[2][MAX_SIDE][MAX_SIDE] = {};
        double point_sum = 0;
        rep(i,h) rep(j,w) states[i][j] = board_state[i][j];
        rep(i,MAX_DEPTH) rep(j,agentNum) if (!no_action[j]) {
            if (actions[j].size() <= i) continue;
            Vec2 target = actions[j][i].target_coord;
            bool is_move = actions[j][i].is_move;
            int y = target.y, x = target.x;
            if (!targeted[is_move][y][x]) {
                int point = 0;
                if (states[y][x] == 0) {
                    if (is_move) {
                        point = points[y][x];
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 1) {
                    if (!is_move) {
                        point = -points[y][x];
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 2) {
                    if (!is_move) {
                        point = points[y][x];
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 3) {
                    if (is_move) {
                        point = points[y][x] - abs(points[y][x]);
                        targeted[is_move][y][x] = true;
                    }
                } else if (states[y][x] == 4) {
                    if (is_move) {
                        point = points[y][x] + abs(points[y][x]);
                        targeted[is_move][y][x] = true;
                    }
                }
                point_sum += point;
            }
        }
//        cout << "eval: " << point_sum << " " << action_count << endl;
//        max_eval = max(max_eval, point_sum / max(1,size));
//        cout << max_eval << endl;
        return point_sum / max(1, size);
    }

    void update_procs(vector<vector<Procedure>> &agents_procs, vector<Procedure> &procs, double &now_eval) {
        int i = xs()%agentNum;
        if (!agents_procs[i].empty()) {
            int j = xs()%agents_procs[i].size();
            auto tmp = procs[i];
            procs[i] = agents_procs[i][j];
            double new_eval = strict_eval(procs);
            if (!permit_transition(new_eval - now_eval)) procs[i] = tmp;
            else now_eval = new_eval;
        }
    }

    vector<Procedure> initial_procs(vector<vector<Procedure>> &agents_procs) {
        vector<Procedure> procs(agentNum);
        rep(i,agentNum) {
            if (!agents_procs[i].empty()) {
                procs[i] = agents_procs[i].front();
            }
        }
        return procs;
    }

    void assign_action(vector<Procedure> procs) {
        rep(i,agentNum) {
            LAction action = procs[i].first_action();
            teams[0].agents[i].action = Action((action.is_move ? MOVE : REMOVE), action.target_coord);
        }
    }

public:
    NextActionDecider() {
        chrono::system_clock::time_point tmp = chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tmp - start).count();

        ms_time_limit = (operationMillis - 1000) - elapsed;
        sc_start = chrono::system_clock::now();
    }
    void decide_next_actions(vector<vector<Procedure>> &agents_procs) {
        vector<Procedure> procs = initial_procs(agents_procs);
        double now_eval = strict_eval(procs);
        now_max_eval = now_eval;
        best_procs = procs;
        int loop_count = 0;
        cout << "first eval: " << now_eval << endl;
        while(is_within_time_limit()) {
            loop_count++;
            update_procs(agents_procs, procs, now_eval);
            if (now_max_eval < now_eval) {
                now_max_eval = now_eval;
                best_procs = procs;
//                cout << now_max_eval << " " << now_eval << endl;
            }
        }
        cout << "loop count: " << loop_count << endl;
        cout << "max eval: " <<  now_max_eval << endl;
        cout << "now eval: " << now_eval << endl;
        assign_action(procs);
    }
};

int main(int argc, char *argv[]) {
    initialize(argc, argv);

    ///*
    ProcedureEnumerator enumerator[MAX_AGENT];
    vector<vector<Procedure>> agents_procs(agentNum);
#pragma omp parallel for num_threads(agentNum)
    rep(i,agentNum) {
        Vec2 now_coord = teams[0].agents[i].coord;
        if (now_coord.x >= 0) {
            agents_procs[i] = enumerator[i].enumerate_procedure(now_coord, 20);
        }
    }

    {
        chrono::system_clock::time_point tmp = chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tmp - start).count();
        cout << elapsed << "[ms]" << endl;
    }

    NextActionDecider decider;
    decider.decide_next_actions(agents_procs);

    rep(i,agentNum) {
        Vec2 now_coord = teams[0].agents[i].coord;
        if (now_coord.x < 0) teams[0].agents[i].action = Action(SET, Vec2(xs()%w, xs()%h));
    }

    {
        chrono::system_clock::time_point tmp = chrono::system_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tmp - start).count();
        cout << elapsed << "[ms]" << endl;
    }

    rep(i,agentNum) {
        double max_eval = INT_MIN;
        double min_eval = INT_MAX;
        for(auto j : agents_procs[i]) {
            max_eval = max(max_eval, j.eval());
            min_eval = min(min_eval, j.eval());
        }
        cout << min_eval << " ~ " << max_eval << endl;
    }
     //*/
    /*
    ProcedureEnumerator enumerator;
    auto v = enumerator.enumerate_procedure(Vec2(12,12), turns - turn);
    int n = min(100, (int)v.size());
    vector<double> e;
    rep(i,n) {
        cout << v[i].eval() << endl;
        v[i].display_proc();
    }
    for(auto i : v) e.push_back(i.eval());
    sort(e.rbegin(), e.rend());
    rep(i,n) cout << e[i] << endl;
    */

    rep(i,agentNum) {
        Vec2 hoge = teams[0].agents[i].coord;
        Action tmp = teams[0].agents[i].action;
        cout << tmp.type << " (" <<  hoge.x << ", " << hoge.y << ") -> (" << tmp.targetCoord.x << ", " << tmp.targetCoord.y << ")" << endl;
    }

    finalize();
}

void initialize(int argc, char *argv[]) {
    start = chrono::system_clock::now();
    if (argc < 2) {
        cerr << "引数が足りません" << endl;
        exit(1);
    }

    initXS(time(NULL));

    matchID = atoi(argv[1]);
    dataPath = (string)(((filesystem::path)(string(argv[0]))).parent_path()) + "/../../data/" + to_string(matchID) + "/";
    input();

    rep(i,h) rep(j,w) {
        if (walls[i][j] == teams[0].teamID) board_state[i][j] = 1;
        if (walls[i][j] == teams[1].teamID) board_state[i][j] = 2;
        if (areas[i][j] == teams[0].teamID) board_state[i][j] = 3;
        if (areas[i][j] == teams[1].teamID) board_state[i][j] = 4;
    }

    rep(i,TEAM_NUM) rep(j,agentNum) {
        Vec2 coord = teams[i].agents[j].coord;
        if (is_safe_coord(coord)) {
            agent_exists[coord.y][coord.x] = true;
        }
    }

    if (turn) {
        int n=0;
        ifstream ifs(dataPath + "prev_action");
        ifs >> n;
        rep(i,n) {
            int idx,x,y;
            string type;
            ifs >> idx >> x >> y >> type;
            if (type == MOVE && teams[0].agents[idx].coord != Vec2(x,y)) conflict_potential[y][x] = true;
            if (type == REMOVE && (board_state[y][x] == 1 || board_state[y][x] == 2)) conflict_potential[y][x] = true;
        }
    }

    set_hyper_parameter();

#ifdef DEBUG
    cout << "solver!" << endl;
    operationMillis = 3000;
#endif
}

void finalize() {
    rep(i,agentNum) teams[1].agents[i].action = Action();

    {
        int n=0;
        ofstream ofs(dataPath + "prev_action");
        rep(i,agentNum) if (!teams[0].agents[i].action.type.empty()) ++n;
        ofs << n << endl;
        rep(i,agentNum) {
            Agent agent = teams[0].agents[i];
            Action action = agent.action;
            if (!agent.action.type.empty()) {
                ofs << i << " " << action.targetCoord.x << " " << action.targetCoord.y << " " << action.type << endl;
            }
        }
    }

    output();
}

bool is_safe_coord(int x, int y) { return 0 <= x && x < w && 0 <= y && y < h; }
bool is_safe_coord(Vec2 coord) { return 0 <= coord.x && coord.x < w && 0 <= coord.y && coord.y < h; }
