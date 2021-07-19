#include "AIController.h"
#include <utility>
#include <bits/stdc++.h>

#pragma GCC optimize("Ofast")
//这不利于调试
using namespace std;
extern int ai_side;
string ai_name = "Stellar Tune";

typedef pair<int, int> Place;
typedef Place Direction;
typedef pair<int, Place> Action;

Action bestaction;

Place place[2];
int boardnum[2];
bool board1[8][8] = {0}, board2[8][8] = {0};
bool can_goboard1[9][8] = {0}, can_goboard2[8][9] = {0};
#define vertical_board board1
#define parallel_board board2

#define Searchdeep 4

string showPlace(Place p) {
    return "{" + to_string(p.first) + "," + to_string(p.second) + "}";
}

string showAction(Action p) {
    return "{" + to_string(p.first) + "," + showPlace(p.second) + "}";
}

void double_search_distance(bool player);

//TODO can_go 使用双表判定

struct StateSaver {
    Place place_s[2];
    int boardnum_s[2];
    bool board1_s[8][8], board2_s[8][8];
    bool can_goboard1_s[9][8], can_goboard2_s[8][9];

    StateSaver() {
        memcpy(place_s, place, sizeof(place));
        memcpy(boardnum_s, boardnum, sizeof(boardnum));
        memcpy(board1_s, board1, sizeof(board1));
        memcpy(board2_s, board2, sizeof(board2));
        memcpy(can_goboard1_s, can_goboard1, sizeof(can_goboard1));
        memcpy(can_goboard2_s, can_goboard2, sizeof(can_goboard2));
        //cout << "save place" << "place[0]=" << showPlace(place[0])<<endl;
        //cout << "save place" << "place[1]=" << showPlace(place[1])<<endl;
    }

    ~StateSaver() {
        memcpy(place, place_s, sizeof(place));
        memcpy(boardnum, boardnum_s, sizeof(boardnum));
        memcpy(board1, board1_s, sizeof(board1));
        memcpy(board2, board2_s, sizeof(board2));
        memcpy(can_goboard1, can_goboard1_s, sizeof(can_goboard1));
        memcpy(can_goboard2, can_goboard2_s, sizeof(can_goboard2));
        //cout << "load place" << "place[0]=" << showPlace(place[0])<<endl;
        //cout << "load place" << "place[1]=" << showPlace(place[1])<<endl;
    }
};

int distmap[9][9], doubledistmap[9][9];//每广搜一次广搜数据就会体现在distmap中。
static constexpr Direction fourdirection[4] = {{1,  0},
                                               {-1, 0},
                                               {0,  1},
                                               {0,  -1}};


Place goodMove(bool player);

int CSV(bool player, int deep, int ub, int lb);

vector<pair<Action, pair<int, int>>> possibleBoard(int player = -1);

int search_distance(bool player);

vector<Place> possibleMove(bool player);

void act(Action loc, bool player);

vector<Action> possibleAction(bool player);

int win();


//init function is called once at the beginning
void init() {
    //ResetClock;

    place[0] = {8, 4};
    place[1] = {0, 4};
    boardnum[0] = boardnum[1] = 10;
    /* Your code here */
}


/* The following notation is based on player 0's perspective
 * Rows are labeled 0 through 8 from player 1's side to player 0's side
 * Columns are labeled 0 through 8 from player 0's left to right
 * A coordinate is recorded as the row followed by the column, for example, player 0's pawn starts on (8, 4)
 * A pawn move is recorded as the new coordinate occupied by the pawn
 * A fence placement is recorded as the coordinate of the square whose bottom-right corner fits the center of the wall
 * A typecode is defined as follows: 0: pawn move
 *                                   1: vertical fence placement
 *                                   2: parallel fence placement
 * An action is defined as (typecode, (row-coordinate, column-coordinate))
 * You need to analyze your opponent's action and return your own action
 * If the typecode of your opponent's action is '-1', it means that you are on the offensive.
 */
int win() {
    //ResetClock;

    if (place[0].first == 0) return 0;
    if (place[1].first == 8) return 1;
    return -1;
}

inline Place Plus(Place old, Direction move) {

    return {old.first + move.first, old.second + move.second};
}

inline bool can_go(Place old, Direction move) {

    if (move.first == 0) {
        if(move.second == 1){
            if(old.second == 8 || can_goboard1[old.first][old.second]) return false;
        }else{
            if(old.second == 0 || can_goboard1[old.first][old.second-1]) return false;
        }
    }else{
        if(move.first == 1){
            if(old.first == 8 || can_goboard2[old.first][old.second]) return false;
        }else{
            if(old.first == 0 || can_goboard2[old.first-1][old.second]) return false;
        }
    }
    return true;
}

void translate(Action &act) {
    //ResetClock;

    if (ai_side != 1)return;
    const int x = 7 + (act.first == 0);
    act.second = {x - act.second.first, x - act.second.second};
}

void double_search_distance(bool player) {
    //ResetClock;

    int dist = search_distance(player);
    memset(doubledistmap, 0x3f, sizeof(doubledistmap));
    doubledistmap[place[player].first][place[player].second] = distmap[place[player].first][place[player].second];
    Place startPlace = place[player];
    queue<Place> q;
    q.push(startPlace);
    while (!q.empty()) {
        startPlace = q.front(), q.pop();
        for (auto move : fourdirection) {
            Place neoPlace = Plus(startPlace, move);
            if (doubledistmap[neoPlace.first][neoPlace.second] > 100 && can_go(startPlace, move) &&
                distmap[neoPlace.first][neoPlace.second] == distmap[startPlace.first][startPlace.second] - 1) {
                doubledistmap[neoPlace.first][neoPlace.second] = distmap[neoPlace.first][neoPlace.second];
                q.push(neoPlace);
            }
        }
    }

}

pair<Action, int> goodBoard(bool player) {
    //ResetClock;

    //顺风过滤，现在只考虑
    //直接返回最优板
    vector<pair<Action, pair<int, int>>> possible_boards = possibleBoard(player);
    int sucker = -1000;
    Action bestBoard;
    if (player == 0)
        for (const auto &possible_board : possible_boards) {
            if (possible_board.second.second - possible_board.second.first > sucker) {
                sucker = possible_board.second.second - possible_board.second.first;
                bestBoard = possible_board.first;
            }
        }
    else
        for (const auto &possible_board : possible_boards) {
            if (possible_board.second.first - possible_board.second.second > sucker) {
                sucker = possible_board.second.first - possible_board.second.second;
                bestBoard = possible_board.first;
            }
        }
    return {bestBoard, sucker};//sucker指放板后对方与我的步数差，越大越好。
}

Place goodMove(bool player) {
    //ResetClock;

    vector<Place> possible_move = possibleMove(player);
    int rundis = 0x3f3f3f3f;
    Place goodmove;
    search_distance(player);
    for (auto m : possible_move) {
        if (distmap[m.first][m.second] < rundis) {
            rundis = distmap[m.first][m.second];
            goodmove = m;
        }
    }
    return goodmove;
}

int son_calculateSituationValue(bool player, int telldistance1) {
    //ResetClock;

    StateSaver s;
    int score = 0;
    /*  // todo 只算4步，之后把板折算了。
      // todo 认为对面只会走路而不会防御性放板的想法是错误的。问题是，防御性放板本身并不增加距离，但是它首先预估阻碍了进攻板的放置（通过封闭空间等规则）（如果加了这步，就提前计算了下一步进攻板的位置，因此如果选择了走路的话就可以不计算下面进攻板的位置了），并且确保不会助纣为虐，看起来不好算
      // todo 放板不会损伤自己，这导致了第一步的那块挡自己的板。
      */

//    int distance1 = search_distance(!player);
    score += ((telldistance1 != -1) ? telldistance1 : search_distance(!player));
    score += boardnum[player];
    if(boardnum[player] <= 5) score += (boardnum[player] - 5) >> 1;
    if(boardnum[player] == 0) score -= 2;
    return score;
}

int calculateSituationValue(
        bool player, int telldistance0 = -1, int telldistance1 = -1) {//FIXME player 是指该谁走了...吧。bug是，应用时我走完了，就直接评估了，而没有考虑他的先手，走完后再评估，即我现在单层应用时应设player=1当然如果把它走的分子全部展开也可以。
//cout << __FUNCTION__ << '\t' << __LINE__ << std::endl;
    //ResetClock;
    int wi = win();
    if (wi == 0) return 10000;
    if (wi == 1) return -10000;
    if (player == 0 && place[0].first == 1 && can_go(place[0], {-1,0})) return 10000;
    if (player == 1 && place[1].first == 7 && can_go(place[1], {1,0})) return -10000;
    //可以根据板的多少加置信度？
    int a0 = son_calculateSituationValue(0,  telldistance1);
    int a1 = son_calculateSituationValue(1, telldistance0);
    return a0 - a1;
}


int measure_with_try(Action aact, bool player, int deep, int ub, int lb){
    StateSaver s;
    act(aact, player);
    if (win() == player) return (player == 0)?10000 : -10000;
    if(deep == 0)    return calculateSituationValue(!player);
    return CSV(!player, deep, ub, lb);
}

int CSV(bool player, int deep, int ub, int lb) {
    //ResetClock;
    if(deep == 0)    return calculateSituationValue(player);

    vector<Action> possible_action = possibleAction(player);
    if (deep == 1) {
        {
            StateSaver s;
            const Action& aact = {0,goodMove(player)};
            act(aact, player);
            int cSV = calculateSituationValue(!player);
            if (player == 0) {
                if (lb < cSV) {
                    lb = cSV;
                }
            } else {
                if (ub > cSV) {
                    ub = cSV;
                }
            }
            if (lb >= ub) return (lb+ub)>>1;//随便return一个值
        }
        for (const auto &aact : possibleBoard(player)) {
            StateSaver s;
            int cSV = calculateSituationValue(!player, aact.second.first, aact.second.second);
            if (player == 0) {
                if (lb < cSV) {
                    lb = cSV;
                }
            } else {
                if (ub > cSV) {
                    ub = cSV;
                }
            }
            if (lb >= ub) return (lb+ub)>>1;
        }

    }
    else {
        //fixme：有没有这一段时间似乎差不多？
        if(deep >= 2){
            //将possible_action 用deep-2削到前10名
            vector<pair<int, Action>> pairvector;
            for (const auto &aact : possible_action) {
                pairvector.push_back({measure_with_try(aact, player, max(0,deep-2), ub, lb),aact});
            }
            if(player == 0)sort(pairvector.rbegin(),pairvector.rend());
            else sort(pairvector.begin(),pairvector.end());
            possible_action.clear();
            for (int i = 0;; ++i) {
                if(i != 0 && pairvector[i].first != pairvector[0].first) break;
                possible_action.push_back(pairvector[i].second);
            }
        }
        for (const auto &aact : possible_action) {
            int val = measure_with_try(aact, player, deep - 1, ub,lb);
            if (player == 0) {
                if (lb < val) {
                    lb = val;
                    if(deep == Searchdeep){
                        bestaction = aact;
                    }
                }
            } else {
                if (ub > val) {
                    ub = val;
                    if(deep == Searchdeep){
                        bestaction = aact;
                    }
                }

            }
            if (lb >= ub) return (lb+ub)>>1;
        }
    }
    return ((player == 0) ? lb : ub);
    // todo 对deep == i， 先用deep == i-1 的结果跑一遍，然后取前10名跑。
}

Action action(Action loc) {
    //ResetClock;

    cerr << loc.first << ' ' << loc.second.first << ' ' << loc.second.second << endl;
    if (loc.first == -1) {
        place[0] = {7, 4};
        return {0, {7, 4}};
    }
    translate(loc);//现在保证你是0了。
    act(loc, 1);
//    StartStopWatch;
    CSV(0, Searchdeep, 1000, -1000);
    Action my_action = bestaction;
//    ResetClock;
    act(my_action, 0);
    translate(my_action);
//    log();
    return my_action;
}

vector<Action> possibleAction(bool player) {
    //ResetClock;

    vector<Action> ret;
    vector<Place> pm = possibleMove(player);
    for (auto i:pm)
        ret.push_back({0, i});
    if (boardnum[player] > 0) {
        auto pb = possibleBoard();
        for (auto i:pb)
            ret.push_back(i.first);
    }
    return ret;
}

vector<Place> possibleMove(bool player) {
    //ResetClock;

    vector<Place> possible_move;

    for (auto move : fourdirection) {
        if (can_go(place[player], move)) {
            if (Plus(place[player], move) == place[!player]) {
                if (can_go(place[!player], move)) possible_move.push_back(Plus(place[!player], move));
                else {
                    swap(move.first, move.second);
                    if (can_go(place[!player], move)) possible_move.push_back(Plus(place[!player], move));
                    move.first = -move.first, move.second = -move.second;
                    if (can_go(place[!player], move)) possible_move.push_back(Plus(place[!player], move));
                }
            } else possible_move.push_back(Plus(place[player], move));
        }
    }
    return possible_move;
}

int search_distance(bool player) {
    //ResetClock;
    Place startPlace;
    queue<Place> q;
    memset(distmap, 0x3f, sizeof(distmap));
    for (int i = 0; i <= 8; ++i) {
        q.push({(player == 0) ? 0 : 8, i});
        distmap[(player == 0) ? 0 : 8][i] = 0;
    }
    while (!q.empty()) {
        startPlace = q.front();
        q.pop();
        if (startPlace == place[player]) return distmap[startPlace.first][startPlace.second];
        for (auto move : fourdirection) {
            Place neoPlace = Plus(startPlace, move);
            if (can_go(startPlace, move) &&
                distmap[neoPlace.first][neoPlace.second] > distmap[startPlace.first][startPlace.second] + 1) {
                distmap[neoPlace.first][neoPlace.second] = distmap[startPlace.first][startPlace.second] + 1;
                q.push(neoPlace);
            }
        }
    }
    return 0x3f3f3f3f;
}

vector<pair<Action, pair<int, int>>> possibleBoard(int player) {
    if (player != -1)
        double_search_distance(!player);
    vector<pair<Action, pair<int, int>>> possible_board;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((i != 0 && vertical_board[i - 1][j]) || (i != 7 && vertical_board[i + 1][j]) ||
                vertical_board[i][j] ||
                parallel_board[i][j])
                continue;
            //问：如何判断路径的完全封锁？
            if (player != -1) {
                if ((doubledistmap[i][j] > 100 || doubledistmap[i][j + 1] > 100) &&
                    (doubledistmap[i + 1][j] > 100 || doubledistmap[i + 1][j + 1] > 100))
                    continue;
            }
            vertical_board[i][j] = true;//try put
            can_goboard1[i][j] = can_goboard1[i+1][j] = true;//try put


            int mydist_after_board = search_distance(0);
            int yourdist_after_board = search_distance(1);
            const bool blocked = mydist_after_board > 100 || yourdist_after_board > 100;
            //这里其实已经广搜把距离数据算出来了，能不能传回去呢？传到后两个数据，用结构体或pair什么的。
            vertical_board[i][j] = false;
            can_goboard1[i][j] = can_goboard1[i+1][j] = false;//try put
            if (blocked) continue;
            possible_board.push_back({{1,                  {i, j}},
                                      {mydist_after_board, yourdist_after_board}});
        }
    }


    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((j != 0 && parallel_board[i][j - 1]) || (j != 7 && parallel_board[i][j + 1]) ||
                parallel_board[i][j] ||
                vertical_board[i][j])
                continue;
            if (player != -1) {
                if ((doubledistmap[i][j] > 100 || doubledistmap[i + 1][j] > 100) &&
                    (doubledistmap[i][j + 1] > 100 || doubledistmap[i + 1][j + 1] > 100))
                    continue;
            }
            parallel_board[i][j] = true;//try put
            can_goboard2[i][j] = can_goboard2[i][j+1] = true;//try put

            int mydist_after_board = search_distance(0);
            int yourdist_after_board = search_distance(1);
            const bool blocked = mydist_after_board > 100 || yourdist_after_board > 100;
            //这里其实已经广搜把距离数据算出来了，能不能传回去呢？传到后两个数据，用结构体或pair什么的。
            parallel_board[i][j] = false;
            can_goboard2[i][j] = can_goboard2[i][j+1] = false;//try put
            if (blocked) continue;
            possible_board.push_back({{2,                  {i, j}},
                                      {mydist_after_board, yourdist_after_board}});
        }
    }

    return possible_board;
}


void act(Action loc, bool player) {
    if (loc.first == 0)
        place[player] = loc.second;
    else {
        --boardnum[player];
        if (loc.first == 1){
            board1[loc.second.first][loc.second.second] = true;
            can_goboard1[loc.second.first][loc.second.second] = true;
            can_goboard1[loc.second.first+1][loc.second.second] = true;

        }else{
            board2[loc.second.first][loc.second.second] = true;
            can_goboard2[loc.second.first][loc.second.second] = true;
            can_goboard2[loc.second.first][loc.second.second+1] = true;
        }
    }
}


/*
// 先写一个最简单的，没有优化风向等等等等的东西，用来debug
//better 可以用关键路径来确定放有些板根本没有改变路径，因而可以。这个优化倍数差不多是2倍左右？

//应当要写出一个减少计算量的局部最短路估值算法，以使得某些时候分析局部就能判断出最短路并无明显变化。
//行挡步差加权基本功估值法
//由某方只放板，某方只走，两边都贪心，直到一方走到终点或一方的放板无法增加距离（有可能吗？）。这时统计放的板数，剩余板数，和走的步数。双方互换，再来一次。
// 考虑步数差或步数比或等等，并判断路的软硬（不容易吧？容易吗？），将板折算为步数（硬路板很无用，软路板很有用），同时，板越稀缺越珍贵。但浪费板换取一格优势也不好。
// 不使用紧急性判定。
//缺点：未考虑对方的反击以及自己的走路。自己在空回合可以走一步？
// 未考虑自己的板对自己的影响。
// 有防御判定机制吗？自己放使对方最优板放不了的板或行走逃跑，对方攻击性放板，看结果如何。
// 未考虑对方和自己在一块区域内时，自己的板对自己的影响。
//2.不能交叉重叠
//3.写一个广搜某点到终点最短距离的。(待优化：搜到终点就不搜了）
 */


/*
//先写纯估值函数贪心，根据对战结果对估值函数进行修正使得基于无分叉贪心O(deep)(还是O（1），说不清)的估值算法也很能打，修正得差不多了以后上搜索树剪枝等方法.
//但是这样的估值函数必须有局部观察能力，不然要跑十几次最短路，相比于直接用最短路当估值函数。希望有最短路快速算法(A*什么的？)。目的：强与快的矛盾平衡
// 先锁定关注目标最短路径，如果连目标最短路径都没影响那就没有意义.
// 把走的方向分为近走和远走。需要根据原先的路径对比。如果一个板使得被迫远走了那么就有阻碍性。这可以快速估值一个板的力量。
// 阻碍产生了以后，贴墙进走。
// 解决集体性入口和绝对性入口。以及入口切换。
//人类是如何观察风向（群）的？对随机板，相邻两步最短路往往共享。关键点/风口的主路径法
// 对点的判定：先沿着这条路径走，看这条路径附近的宽阔度，是否有板。如果有，做局部观察，看在局部削掉指定板后是否走更少的步数就能回到路径上的某点。
// 以上都试过不行再来广搜。每一步都维护最短路径？
// 顺风搜，扶墙搜，相比与广搜
// 一定注意自己下一步的风向变化，基本上就是大变化。如果走几步能回到原来的风向那还好。
// 这都是自己的走法，对别人呢？问，怎么很好地根据风向论低运算量启发式地档别人？关键：控制风向双风或大变，或基本功。
// 可以概率地使用一次真最短路，频率根据搜索的重要性而定。启发式等。
// 尽可能多地保存状态及算数据，这样到了之后的回合也能用，很好。之后再做就不是新建而是修改了。对风向强控制技巧看来要一点分支决策？
// 加一个板以后刷周围的密集度。制作两个风向表，两个密度表什么的。如果要用Floyd的话，那计算的就是全局所有点针对终点的风向了，这个可以接收到以后计算一次。
// 得到数据后，分为两种计算，一次计算和分支计算。一次计算算出的数据量可以全面一点。
// 如果某次放板改变了某些地方的风向，可以对比然后关注这些点。同时尤其关注自己相关的点的风向。
// 同时应关注完全不同的路径分支，以作为次级关注点。并通过切分支，打分支战来攻击对方。
// 关于主动攻击：如何做到分支诱敌部署？
//以上有一部分是局面最优，有一部分是动作最优。但是动作最优可以利用差分局部性，局面最优不能？
// 因此有动作估值和局面估值两条路。
//把某一步的收益折算为步数,只用管步数差不用管步数比吧。
// 广搜还是可以的，但要考虑优化，比如双向广搜或者注意力机制，不过感觉都是常数就是了。
// 怎么找到最优放板地点——枚举关注路径上的板，判断路径增加数？和最优挡板地点？
 */


// todo 逆搜+正搜确定路径，不同距离分层就很好。然后依次就可以测定板到底有没有用，减少计算量。

// 现在写的所有东西都是价值评估函数
