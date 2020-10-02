#include <bits/stdc++.h>
#include "constant.hpp"
#include "classes.hpp"
using namespace std;

extern int w,h;

bool isSafeIdx(Vec2 coord) { return 0 <= coord.x && coord.x < w && 0 <= coord.y && coord.y < h; }

