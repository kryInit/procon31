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

int dist2p(Vec2 s, Vec2 t);
int m_insert_cost(Vec2 prev, Vec2 target, Vec2 next);
int f_insert_cost(Vec2 target, Vec2 next);
int b_insert_cost(Vec2 prev, Vec2 target);
int m_erase_cost(Vec2 prev, Vec2 target, Vec2 next);
int f_erase_cost(Vec2 target, Vec2 next);
int b_erase_cost(Vec2 prev, Vec2 target);

bool is_safe_coord(int x, int y);
bool is_safe_coord(Vec2 coord);

int h, w, agentNum, turns, turn, operationMillis;
int walls[MAX_SIDE][MAX_SIDE], areas[MAX_SIDE][MAX_SIDE], points[MAX_SIDE][MAX_SIDE];
Team teams[TEAM_NUM];

int dist[MAX_SIDE][MAX_SIDE][MAX_SIDE][MAX_SIDE];
int actual_points[MAX_SIDE][MAX_SIDE];

const int d[3] = {-1,0,1};

int matchID;
string dataPath;
clock_t start;

class Order {
    int min_order_size=0, max_order_size=576;
    list<Vec2> order;
    int total_point=0, total_dist=0;
    Vec2 start_coord = Vec2(-1,-1);

    bool placed() { return start_coord.x >= 0; }

    double __objective(int tp, int td) {
        if (td == 0) return 0;
        else return (double)tp / (double)td;
    }
    int __insertion(Vec2 target, bool need_exec) {
        if (order.empty() || order.size() == max_order_size) {
            if (placed()) {
                int d_dist = b_insert_cost(start_coord, target);
                if (need_exec) {
                    total_dist += d_dist;
                    total_point += actual_points[target.y][target.x];
                    order.push_front(target);
                }
                return d_dist;
            }
            else {
                if (need_exec) {
                    total_dist += 2;
                    total_point += actual_points[target.y][target.x];
                    order.push_front(target);
                }
                return 2;
            }
        }
        auto best_itr = order.end();
        int min_d_dist=1000000;
        for(auto itr = order.begin();;++itr) {
            int tmp_d_dist;
            if (itr == order.begin()) {
                Vec2 next = *itr;
                if (placed()) tmp_d_dist = m_insert_cost(start_coord, target, next);
                else tmp_d_dist = f_insert_cost(target, next);
            } else if (itr == order.end()) {
                Vec2 prev = *--itr;++itr;
                tmp_d_dist = b_insert_cost(prev, target);
            } else {
                Vec2 prev = *--itr;++itr;
                Vec2 next = *itr;
                tmp_d_dist = m_insert_cost(prev, target, next);
            }
            if (min_d_dist > tmp_d_dist) {
                min_d_dist = tmp_d_dist;
                best_itr = itr;
            }
            if (itr == order.end()) break;
        }
        if (need_exec) {
            total_point += actual_points[target.y][target.x];
            total_dist += min_d_dist;
            order.insert(best_itr, target);
        }
        return min_d_dist;
    }
    int __erasing(Vec2 target, bool need_exec) {
        if (order.empty() || order.size() <= min_order_size) return 0;
        if (order.size() == 1) {
            if (order.front() == target) {
                if (placed()) {
                    int d_dist = b_erase_cost(start_coord, target);
                    if (need_exec) {
                        total_point -= actual_points[target.y][target.x];
                        total_dist += d_dist;
                        order.clear();
                    }
                    return d_dist;
                } else {
                    int d_dist = -2;
                    if (need_exec) {
                        total_point -= actual_points[target.y][target.x];
                        total_dist += d_dist;
                        order.clear();
                    }
                    return d_dist;
                }
            }
            return 0;
        }
        for(auto itr = order.begin(); itr != order.end(); ++itr) {
            if (*itr == target) {
                int d_dist;
                if (itr == order.begin()) {
                    Vec2 next = *++itr;--itr;
                    if (placed()) d_dist = m_erase_cost(start_coord, target, next);
                    else d_dist = f_erase_cost(target, next);
                } else if (itr == --order.end()) {
                    Vec2 prev = *--itr;++itr;
                    d_dist = b_erase_cost(prev, target);
                } else {
                    Vec2 prev = *--itr;++itr;
                    Vec2 next = *++itr;--itr;
                    d_dist = m_erase_cost(prev, target, next);
                }
                if (need_exec) {
                    total_point -= actual_points[target.y][target.x];
                    total_dist += d_dist;
                    order.erase(itr);
                }
                return d_dist;
            }
        }
        return 0;
    }
    int __replacement(Vec2 oldc, Vec2 newc, bool need_exec) {
        if (order.empty()) return 0;
        if (order.size() == 1) {
            if (order.front() == oldc) {
                if (placed()) {
                    int d_dist = b_erase_cost(start_coord, oldc) + b_insert_cost(start_coord, newc);
                    if (need_exec) {
                        total_point += actual_points[newc.y][newc.x] - actual_points[oldc.y][oldc.x];
                        total_dist += d_dist;
                        *order.begin() = newc;
                    }
                    return d_dist;
                } else {
                    int d_dist = 0;
                    if (need_exec) {
                        total_point += actual_points[newc.y][newc.x] - actual_points[oldc.y][oldc.x];
                        total_dist += d_dist;
                        *order.begin() = newc;
                    }
                    return d_dist;
                }
            }
            return 0;
        }
        for(auto itr = order.begin(); itr != order.end(); ++itr) {
            if (*itr == oldc) {
                int d_dist;
                if (itr == order.begin()) {
                    Vec2 next = *++itr;--itr;
                    if (placed()) d_dist = m_erase_cost(start_coord, oldc, next) + m_insert_cost(start_coord, newc, next);
                    else d_dist = f_erase_cost(oldc, next) + f_insert_cost(newc, next);
                } else if (itr == --order.end()) {
                    Vec2 prev = *--itr;++itr;
                    d_dist = b_erase_cost(prev, oldc) + b_insert_cost(prev, newc);
                } else {
                    Vec2 prev = *--itr;++itr;
                    Vec2 next = *++itr;--itr;
                    d_dist = m_erase_cost(prev, oldc, next) + m_insert_cost(prev, newc, next);
                }
                if (need_exec) {
                    total_point += actual_points[newc.y][newc.x] - actual_points[oldc.y][oldc.x];
                    total_dist += d_dist;
                    *itr = newc;
                }
                return d_dist;
            }
        }
        return 0;
    }

public:
    Order() {}
    Order(Vec2 _start_coord) { start_coord = _start_coord; }
    Order(int _min_order_size, int _max_order_size) {
        min_order_size = _min_order_size;
        max_order_size = _max_order_size;
    }
    Order(int _min_order_size, int _max_order_size, Vec2 _start_coord) {
        min_order_size = _min_order_size;
        max_order_size = _max_order_size;
        start_coord = _start_coord;
    }

    double objective() {
        return __objective(total_point, total_dist);
    }
    double dobjective(int dp, int dd) {
        return __objective(total_point + dp, total_dist + dd) - __objective(total_point, total_dist);
    }

    void insert(Vec2 target) {
        __insertion(target, true);
    }
    void erase(Vec2 target) {
        __erasing(target, true);
    }
    void replace(Vec2 oldc, Vec2 newc) {
        __replacement(oldc, newc, true);
    }
    int dinsert(Vec2 target) {
        return __insertion(target, false);
    }
    int derase(Vec2 target) {
        return __erasing(target, false);
    }
    int dreplace(Vec2 oldc, Vec2 newc) {
        return __replacement(oldc, newc, false);
    }

    void final_update() {
        auto v = get_order();
        for(auto i : v) {
            int d_dist = derase(i);
            int d_point = -actual_points[i.y][i.x];
            if (dobjective(d_point, d_dist) > 0) erase(i);
        }
    }
    vector<Vec2> get_order() {
        vector<Vec2> v_order;
        for(auto i : order) v_order.push_back(i);
        return v_order;
    }

    bool tooSmall() { return order.size() < min_order_size; }
    bool justSize() { return order.size() == min_order_size; }
    bool isLimitSize() { return order.size() == max_order_size; }
};

class OperateOrder {
    Order order[MAX_AGENT];
    clock_t birth_time, time_limit;

    const double TEMP_RADIX = 1000;

    double tempreture() {
        return pow(TEMP_RADIX, (double)(clock()-birth_time) / (double)time_limit);
    }
    double probability(double diff, double t) {
        if (diff > 0) return 1;
        else return exp(diff*t*50);
    }
    bool permit_transition(double diff) {
        return probability(diff, tempreture()) > ((double)xs() / (double)UINT_MAX);
    }

public:
    OperateOrder() {
        rep(i,agentNum) {
            Vec2 my_coord = teams[0].agents[i].coord;
            if (is_safe_coord(my_coord)) {
                order[i] = Order(my_coord);
            }
        }
        birth_time = clock();
        time_limit = (start + (operationMillis-1000) / 1000. * CLOCKS_PER_SEC) - birth_time;
    }
    OperateOrder(vector<int> min_order_sizes) {
        rep(i,agentNum) {
            Vec2 my_coord = teams[0].agents[i].coord;
            if (is_safe_coord(my_coord)) {
                int l = 5;
                int r = min(20, max(l+1, min_order_sizes[i]));
                order[i] = Order(l, r, my_coord);
            } else {
                int l = 5;
                int r = min(20, max(l+1, min_order_sizes[i]));
                order[i] = Order(l, r);
            }
        }
        birth_time = clock();
        time_limit = (start + (operationMillis-1000) / 1000. * CLOCKS_PER_SEC) - birth_time;
    }

    void insert(int idx, Vec2 target, int who_took[MAX_SIDE][MAX_SIDE]) {
        if (order[idx].tooSmall()) {
            order[idx].insert(target);
            who_took[target.y][target.x] = idx;
            return;
        } if (order[idx].isLimitSize()) return;
        int d_dist = order[idx].dinsert(target);
        int d_point = actual_points[target.y][target.x];
        double d_objective = order[idx].dobjective(d_point, d_dist);
        if (permit_transition(d_objective)) {
            order[idx].insert(target);
            who_took[target.y][target.x] = idx;
        }
    }
    void erase(int idx, Vec2 target, int who_took[MAX_SIDE][MAX_SIDE]) {
        if (order[idx].justSize() || order[idx].tooSmall()) return;
        int d_dist = order[idx].derase(target);
        int d_point = -actual_points[target.y][target.x];
        double d_objective = order[idx].dobjective(d_point, d_dist);
        if (permit_transition(d_objective)) {
            order[idx].erase(target);
            who_took[target.y][target.x] = -1;
        }
    }
    void replace(int idx, Vec2 oldc, Vec2 newc, int who_took[MAX_SIDE][MAX_SIDE]) {
        int d_dist = order[idx].dreplace(oldc, newc);
        int d_point = actual_points[newc.y][newc.x] - actual_points[oldc.y][oldc.x];
        double d_objective = order[idx].dobjective(d_point, d_dist);
        if (permit_transition(d_objective)) {
            order[idx].replace(oldc, newc);
            who_took[oldc.y][oldc.x] = -1;
            who_took[newc.y][newc.x] = idx;
        }
    }
    void swap(int prev_idx, int next_idx, Vec2 target, int who_took[MAX_SIDE][MAX_SIDE]) {
        if (order[prev_idx].justSize() || order[prev_idx].tooSmall()) return;
        if (order[next_idx].isLimitSize()) return;
        double d_objective = 0, d_dist, d_point;

        d_dist = order[prev_idx].derase(target);
        d_point = -actual_points[target.y][target.x];
        d_objective += order[prev_idx].dobjective(d_point, d_dist);

        d_dist = order[next_idx].dinsert(target);
        d_point *= -1;
        d_objective += order[next_idx].dobjective(d_point, d_dist);

        if (permit_transition(d_objective)) {
            order[prev_idx].erase(target);
            order[next_idx].insert(target);
            who_took[target.y][target.x] = next_idx;
        }
    }
    void swap(int idxL, Vec2 targetL, int idxR, Vec2 targetR, int who_took[MAX_SIDE][MAX_SIDE]) {
        double d_objective = 0, d_dist, d_point;

        d_dist = order[idxL].dreplace(targetL, targetR);
        d_point = actual_points[targetR.y][targetR.x] - actual_points[targetL.y][targetL.x];
        d_objective += order[idxL].dobjective(d_point, d_dist);

        d_dist = order[idxR].dreplace(targetR, targetL);
        d_point = actual_points[targetL.y][targetL.x] - actual_points[targetR.y][targetR.x];
        d_objective += order[idxR].dobjective(d_point, d_dist);

        if (permit_transition(d_objective)) {
            order[idxL].replace(targetL, targetR);
            order[idxR].replace(targetR, targetL);
            who_took[targetL.y][targetL.x] = idxR;
            who_took[targetR.y][targetR.x] = idxL;
        }
    }
    vector<Vec2> get_order(int idx) { return order[idx].get_order(); }
    double get_objective(int idx) { return order[idx].objective(); }
    bool can_operate() { return time_limit + birth_time > clock(); }
    void final_update() { rep(i,agentNum) order[i].final_update(); }
};

class Solver {
    const int default_min_order_size = 20;
    vector<vector<Vec2>> orders;
    void build_order() {
        int who_took[MAX_SIDE][MAX_SIDE] = {};
        vector<Vec2> first_coord = decide_first_coord();
        priority_queue<pair<double,Vec2>> tile_pq[MAX_AGENT];
        priority_queue<pair<double,int>, vector<pair<double,int>>, greater<pair<double,int>>> idx_pq;

        rep(i,h) rep(j,w) who_took[i][j] = -1;
        rep(i,agentNum) {
            Vec2 my_coord = first_coord[i];
            int y = my_coord.y, x = my_coord.x;
            if (teams[0].agents[i].coord == my_coord) who_took[y][x] = i;
            for(int Y = max(0,y-2); Y < min(y+3, h); ++Y) {
                for(int X = max(0, x-2); X < min(x+3, w); ++X) {
                    Vec2 target = Vec2(X,Y);
                    double criteria;
                    if (Y == y && X == x) criteria = actual_points[Y][X] / 2.;
                    else criteria = actual_points[Y][X] / dist2p(my_coord, target);
                    tile_pq[i].push(make_pair(criteria, target));
                }
            }
        }


        vector<int> min_order_sizes = decide_min_order_size();
        OperateOrder order_operator(min_order_sizes);

        rep(i,agentNum) if (teams[0].agents[i].coord.x < 0) {
            order_operator.insert(i, first_coord[i], who_took);
        }
        rep(i,agentNum) idx_pq.push(make_pair(order_operator.get_objective(i), i));

        int loop_count = 0;
        int cnt[MAX_AGENT] = {};
        while(order_operator.can_operate()) {
            loop_count++;
            int idx = idx_pq.top().second; idx_pq.pop();
            int y = xs()%h, x = xs()%w;
            if (!tile_pq[idx].empty()) {
                y = tile_pq[idx].top().second.y;
                x = tile_pq[idx].top().second.x;
                tile_pq[idx].pop();
            }
            Vec2 target(x,y);
            if (target == teams[0].agents[idx].coord) {
                cnt[idx]++;
                idx_pq.push(make_pair(order_operator.get_objective(idx)*cnt[idx], idx));
                continue;
            }
            if (who_took[y][x] == -1) {
                order_operator.insert(idx, target, who_took);
                if (who_took[y][x] == idx) {
                    for(int Y = max(0,y-2); Y < min(y+3, h); ++Y) {
                        for(int X = max(0, x-2); X < min(x+3, w); ++X) {
                            Vec2 next = Vec2(X,Y);
                            double criteria;
                            if (Y == y && X == x) criteria = actual_points[Y][X] / 2.;
                            else criteria = actual_points[Y][X] / dist2p(target, next);
                            tile_pq[idx].push(make_pair(criteria, next));
                        }
                    }
                }
            }
            else if (who_took[y][x] == idx) {
                order_operator.erase(idx, target, who_took);
                rep(i,3) rep(j,3) if ((i != 1 || j != 1) && who_took[y][x] == idx){
                    int Y = y+d[i], X = x+d[j];
                    Vec2 additional_target = Vec2(X,Y);
                    if (is_safe_coord(additional_target) && additional_target != teams[0].agents[idx].coord) {
                        if (who_took[Y][X] == -1) order_operator.replace(idx, target, additional_target, who_took);
                        else if (additional_target != teams[0].agents[who_took[Y][X]].coord) order_operator.swap(idx, target, who_took[Y][X], additional_target, who_took);
                    }
                }
            }
            else {
                if (target != teams[0].agents[who_took[y][x]].coord) {
                    order_operator.swap(who_took[y][x], idx, target, who_took);
                    if (who_took[y][x] == idx) {
                        for(int Y = max(0,y-2); Y < min(y+3, h); ++Y) {
                            for(int X = max(0, x-2); X < min(x+3, w); ++X) {
                                Vec2 next = Vec2(X,Y);
                                double criteria;
                                if (Y == y && X == x) criteria = actual_points[Y][X] / 2.;
                                else criteria = actual_points[Y][X] / dist2p(target, next);
                                tile_pq[idx].push(make_pair(criteria, next));
                            }
                        }
                    }
                }
            }
            cnt[idx]++;
            idx_pq.push(make_pair(order_operator.get_objective(idx)*cnt[idx], idx));
        }
        order_operator.final_update();
        rep(i,agentNum) orders.push_back(order_operator.get_order(i));
        ///*
        cout << "loop count: " << loop_count << endl;
        rep(i,agentNum) cout << cnt[i] << endl;
        rep(i,agentNum) {
            cout << order_operator.get_objective(i) << endl;
        }

        int f[MAX_SIDE][MAX_SIDE] = {};
        char c[MAX_SIDE][MAX_SIDE] = {};
        rep(i,h) rep(j,w) c[i][j] = ' ';
        rep(i,agentNum) {
            auto v = order_operator.get_order(i);
            for(auto j : v) {
                if (f[j.y][j.x]) {
                    cout << "ERROR: same tile is included!" << endl;
                    exit(1);
                }
                f[j.y][j.x] = true;
                cout << "(" << j.x << ", " << j.y << ") -> ";
            }
            cout << "end" << endl;
            for(auto j : v) c[j.y][j.x] = i+'a';
            if (v.size()) c[v[0].y][v[0].x] = i+'A';
        }
        rep(i,h) {
            rep(j,w) cout << c[i][j] << " ";
            cout << endl;
        }

        rep(i,h) {
            rep(j,w) {
                if (who_took[i][j] == -1) cout << '.' << c[i][j] << " ";
                else cout << (char)(who_took[i][j]+'a') << c[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
         //*/
    }
    void expectation() {

    }
    void decide_action() {
        rep(k,agentNum) {
            Vec2 now = teams[0].agents[k].coord;
            vector<Vec2> order = orders[k];
            if (is_safe_coord(now)) {
                if (order.size()) {
                    Vec2 target = order.front();
                    if (max(abs(now.y-target.y), abs(now.x-target.x)) <= 1) {
                        if (walls[target.y][target.x] == teams[0].teamID) {
                            if (order.size() > 1) {
                                if (dist2p(now, order[1]) > dist2p(now, target) + dist2p(target, order[1])) {
                                    teams[0].agents[k].action = Action(MOVE, target);
                                } else {
                                    teams[0].agents[k].action = Action(REMOVE, target);
                                }
                            }
                        } else if (walls[target.y][target.x] == teams[1].teamID) {
                            teams[0].agents[k].action = Action(REMOVE, target);
                        } else teams[0].agents[k].action = Action(MOVE, target);
                    } else {
                        pair<int,int> best_dp = make_pair(INT_MAX, 0);
                        Vec2 next;
                        int y = now.y, x = now.x;
                        rep(i,3) rep(j,3) if (i != 1 || j != 1) {
                            int Y = y+d[i], X = x+d[j];
                            Vec2 tmp = Vec2(X,Y);
                            if (is_safe_coord(tmp)) {
                                int tmp_dist = dist2p(now, tmp) + dist2p(tmp, next);
                                int tmp_point = (walls[i][j] == teams[0].teamID ? 0 : actual_points[i][j]);
                                if (best_dp.first > tmp_dist) {
                                    best_dp.first = tmp_dist;
                                    best_dp.second = tmp_point;
                                    next = tmp;
                                } else if (best_dp.first == tmp_dist && best_dp.second < tmp_point) {
                                    best_dp.second = tmp_point;
                                    next = tmp;
                                }
                            }
                        }
                        string type = (walls[next.y][next.x] == teams[1].teamID ? REMOVE : MOVE);
                        teams[0].agents[k].action = Action(type, next);
                    }
                } else teams[0].agents[k].action = Action("stay", teams[0].agents[k].coord);
            } else {
                if (order.size()) {
                    pair<int,int> best_dp = make_pair(INT_MAX, 0);
                    Vec2 target = order.front();
                    Vec2 next;
                    rep(i,h) rep(j,w) {
                        if ((i != target.y || j != target.x) && walls[i][j] != teams[1].teamID) {
                            int tmp_dist = dist2p(Vec2(j,i), target);
                            int tmp_point = (walls[i][j] == teams[0].teamID ? 0 : actual_points[i][j]);
                            if (best_dp.first > tmp_dist) {
                                best_dp.first = tmp_dist;
                                best_dp.second = tmp_point;
                                next = Vec2(j,i);
                            } else if (best_dp.first == tmp_dist && best_dp.second < tmp_point) {
                                best_dp.second = tmp_point;
                                next = Vec2(j,i);
                            }
                        }
                    }
                    teams[0].agents[k].action = Action(SET, next);
                }
            }
        }
    }

    vector<int> decide_min_order_size() {
        int dist_from_agent[MAX_AGENT][2][MAX_AGENT] = {};
        rep(i,agentNum) rep(j,2) rep(k,agentNum) dist_from_agent[i][j][k] = INT_MAX;
        rep(i,agentNum) {
            Vec2 my_coord = teams[0].agents[i].coord;
            if (is_safe_coord(my_coord)) {
                rep(j,2) rep(k,agentNum) {
                    if (j == 0 && i == k) continue;
                    Vec2 another_coord = teams[j].agents[k].coord;
                    if (is_safe_coord(another_coord)) {
                        dist_from_agent[i][j][k] = dist2p(my_coord, another_coord);
                    }
                }
            }
        }
        vector<int> result(agentNum);
        rep(i,agentNum) {
            Vec2 my_coord = teams[0].agents[i].coord;
            if (is_safe_coord(my_coord)) {
                int min_dist[2] = {INT_MAX, INT_MAX};
                rep(j,2) rep(k,agentNum) min_dist[j] = min(min_dist[j], dist_from_agent[i][j][k]);
//                result[i] = min(default_min_order_size, max(min_dist[0], min_dist[1]));
                result[i] = min(default_min_order_size, min_dist[1]);
            } else {
                result[i] = default_min_order_size;
            }
            cout << result[i]<< endl;
        }
        return result;
    }
    vector<Vec2> decide_first_coord() {
        // 基本的には未配置エージェント用
        // 配置済みエージェントは配置場所を返すようにしてる
        vector<Vec2> coords(MAX_SIDE);
        bool took[MAX_SIDE][MAX_SIDE] = {};
        int dist_from_agent[MAX_SIDE][MAX_SIDE] = {};
        rep(i,h) rep(j,w) dist_from_agent[i][j] = INT_MAX;
        rep(k,agentNum) {
            Vec2 my_coord = teams[0].agents[k].coord;
            if (is_safe_coord(my_coord)) {
                rep(i,h) rep(j,w) dist_from_agent[i][j] = min(dist_from_agent[i][j], dist2p(my_coord,Vec2(j,i))*actual_points[i][j]);
                coords[k] = my_coord;
                took[my_coord.y][my_coord.x] = true;
            }
        }
        rep(k,agentNum) {
            Vec2 my_coord = teams[0].agents[k].coord;
            if (!is_safe_coord(my_coord)) {
                int now_max = INT_MIN;
                Vec2 coord;
                rep(I,h*w) {
                    int i = I/w, j = I%w;
                    if (dist_from_agent[i][j] == INT_MAX) {
                        coord = Vec2(xs()%w, xs()%h);
                        break;
                    }
                    if (now_max < dist_from_agent[i][j] && !took[i][j]) {
                        now_max = dist_from_agent[i][j];
                        coord = Vec2(j,i);
                    }
                }
                coords[k] = coord;
                took[coord.y][coord.x] = true;
                rep(i,h) rep(j,w) dist_from_agent[i][j] = min(dist_from_agent[i][j], dist2p(coord,Vec2(j,i))*actual_points[i][j]);
            }
        }
//        rep(i,agentNum) cout << "(" << coords[i].y << ", " << coords[i].x << ")" << endl;
        return coords;
    }

public:
    void solve() {
        build_order(); // タイルの取得順を作る
        expectation(); // 敵の行動を予測する
        decide_action(); // タイルの取得順と敵の行動を見て次の行動を決める
    }
};


int main(int argc, char *argv[]) {
    initialize(argc, argv);

    Solver solver;
    solver.solve();

    rep(i,agentNum) {
        Vec2 hoge = teams[0].agents[i].coord;
        Action tmp = teams[0].agents[i].action;
        cout << tmp.type << " (" <<  hoge.x << ", " << hoge.y << ") -> (" << tmp.targetCoord.x << ", " << tmp.targetCoord.y << ")" << endl;
    }

    finalize();
}

int calcDistWithPoint(Vec2 coord) {
    int y = coord.y, x = coord.x;
    int result;
    if (areas[y][x] == teams[0].teamID) {
        if (points[y][x] < 0) result = 3;
        else result = 1;
    }
    else if (areas[y][x] == teams[1].teamID) result = 2;
    else if (walls[y][x] == teams[0].teamID) result = 1;
    else if (walls[y][x] == teams[1].teamID) {
        if (points[y][x] < 0) result = 4;
        else result = 2;
    }
    else result = 2;
    return result;
}

void calcDists() {
    struct state {
        Vec2 to;
        int dist;
        state(){to=Vec2();dist=1e5;};
        state(Vec2 _to, int _dist) {
            to = _to;
            dist = _dist;
        }
        bool operator<( const state& another ) const {
            return dist > another.dist;
        }
    };

#pragma omp parallel for
    rep(si,h) rep(sj,w) {
        priority_queue<state> pq;
        bool visited[MAX_SIDE][MAX_SIDE] = {};
        pq.push(state(Vec2(sj,si), 0));
        while(pq.size()) {
            int _dist=pq.top().dist;
            Vec2 now=pq.top().to;
            pq.pop();
            if (visited[now.y][now.x]) continue;
            visited[now.y][now.x] = true;
            dist[si][sj][now.y][now.x] = _dist;
            rep(i,3) rep(j,3) {
                Vec2 to(now.x+d[i], now.y+d[j]);
                if (is_safe_coord(to) && !visited[to.y][to.x]) {
                    int dDist = calcDistWithPoint(to);
                    pq.push(state(to, _dist+dDist));
                }
            }
        }
    }
}

int dist2p(Vec2 s, Vec2 t) {
    if (s == t) return 0;
    int min_dist = 1000000;
    rep(i,3) rep(j,3) {
        if (i == 1 && j == 1) continue;
        int y = t.y+d[i], x = t.x+d[j];
        if (is_safe_coord(x,y)) {
            min_dist = min(min_dist, dist[s.y][s.x][y][x]);
        }
    }
    return min_dist + (walls[t.y][t.x] == teams[1].teamID ? 2 : 1);
};
int m_insert_cost(Vec2 prev, Vec2 target, Vec2 next) {
    return dist2p(prev, target) + dist2p(target, next) - dist2p(prev, next);
}
int f_insert_cost(Vec2 target, Vec2 next) {
    return dist2p(target, next);
}
int b_insert_cost(Vec2 prev, Vec2 target) {
    return dist2p(prev, target);
}
int m_erase_cost(Vec2 prev, Vec2 target, Vec2 next) {
    return -m_insert_cost(prev, target, next);
}
int f_erase_cost(Vec2 target, Vec2 next) {
    return -f_insert_cost(target, next);
}
int b_erase_cost(Vec2 prev, Vec2 target) {
    return -b_insert_cost(prev, target);
}


void initialize(int argc, char *argv[]) {
    start = clock();
    if (argc < 2) {
        cerr << "引数が足りません" << endl;
        exit(1);
    }

    initXS(time(NULL));

    matchID = atoi(argv[1]);
    dataPath = (string)(((filesystem::path)(string(argv[0]))).parent_path()) + "/../../data/" + to_string(matchID) + "/";
    input();

    rep(i,h) rep(j,w) {
        int point = 0;
        if (areas[i][j] == teams[0].teamID) point = points[i][j] - abs(points[i][j]);
        else if (areas[i][j] == teams[1].teamID) point = points[i][j] + abs(points[i][j]);
        else if (walls[i][j] == teams[0].teamID) point = -points[i][j];
        else if (walls[i][j] == teams[1].teamID) point = points[i][j]*2;
        else point = points[i][j];
        actual_points[i][j] = point;
    }

    chrono::system_clock::time_point tmps, tmpe;
    tmps = chrono::system_clock::now();
    calcDists();
    tmpe = chrono::system_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tmpe-tmps).count();
    cout << elapsed / 1000. << "[ms]" << endl;

#ifdef DEBUG
    cout << "solver!" << endl;
    operationMillis = 3000;
#endif
}

void finalize() {
    rep(i,agentNum) teams[1].agents[i].action = Action();
    output();
}

bool is_safe_coord(int x, int y) { return 0 <= x && x < w && 0 <= y && y < h; }
bool is_safe_coord(Vec2 coord) { return 0 <= coord.x && coord.x < w && 0 <= coord.y && coord.y < h; }

/*
 * 距離の定義
 * 必須 + 元に戻す
 * area == my teamID:
 *      if point < 0: 1 + 2
 *      else        : 1 + 0
 * aera == op teamID: 1 + 1
 * wall == my teamID: 1 + 0
 * wall == op teamID:
 *      if point < 0: 2 + 2
 *      else        : 2 + 0
 * otherwise        : 1 + 1
 * */

/*
 * スコアの定義
 * area == my teamID: point - abs(point)
 * area == op teamID: point + abs(point)
 * wall == my teamID: -point
 * wall == op teamID: point*2
 * else             : point
 * */

