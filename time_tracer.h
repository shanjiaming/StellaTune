//
// Created by JiamingShan on 2021/2/16.
//

#ifndef CODE_LOGGER_H
#define CODE_LOGGER_H
#include <iostream>
#include <fstream>

//这是整个TimeTracing功能的开关。即使在程序中使用了TimeTracing的宏，只需关掉此开关即可让它们失效。
#define TimeTracing

/*
 * 这个头文件用来做各函数的时间测量。
 * #include "time_tracer.h"
 * 然后使用其功能。
 *
 * 文件中有一个唯一的时钟在不停地走，并且任意时刻时钟都关联于一个函数名。
 * 提供的功能有:
 *
 * ResetClock;
 * 这个语句在某个函数中插入，则时钟把它的此刻读数加入上一个函数的总计时，时钟置0，并关联当前函数的名字。
 * 最经常的用法是在主要函数的第一行插入。
 *
 * StartStopWatch;
 * 这个语句在某个函数中插入，则时钟把它的此刻读数加入上一个函数的总计时，时钟置0，并关联名为“StopWatch"的特殊名字。
 *
 * 所有结果输入了log.dat文件。你可以直接在任意时刻触发log（）函数，则结果会被打印至控制台（推荐）。触发log()后，文件内容将被清空
 *
 * 若注释掉log()最后两行，则log.dat将在每次运行log（）函数后保留而非清空。这样你可以在程序运行完毕后直接打开该文件观看其记录结果。
 *
 */

/* 样例程序：
#include <iostream>
#include "time_tracer.h"
using namespace std;

int my_pow(int x, int n){
    ResetClock;
    int ans = 1;
    while(n--) ans*=x;
    return ans;
}

int main(){
    int x = 3; int n = 30;
    int N = 1000;
    while(N--){
        StartStopWatch;
        cout << my_pow(x,n) << endl;
    }
    log();
    return 0;
}
 */

#ifdef TimeTracing
#include <ctime>
#include <map>
inline auto& main_log(){ static std::ofstream main_log("log.dat", std::ios::app); return main_log;}
inline auto& time_recorder(){ static std::map<const char *, int> time_recorder; return time_recorder;}
inline auto& function_called_num(){ static std::map<const char *, int> function_called_num; return function_called_num;}
inline auto& functionToBeTimed(){ static int *functionToBeTimed = nullptr; return functionToBeTimed;}
inline auto& tClockTimer(){ static clock_t tClockTimer = clock(); return tClockTimer;}
#define LocalClock(x)   do{if(functionToBeTimed())*functionToBeTimed() += clock() - tClockTimer();tClockTimer() = clock();functionToBeTimed() = &time_recorder()[x];++function_called_num()[x];}while(0)
#define ResetClock     LocalClock(__FUNCTION__)
#define StartStopWatch LocalClock("STOP_WATCH")
#define DisplayClock   for(auto f_t:time_recorder()) main_log() << std::string("                      " ).replace(0,std::string(f_t.first).length(),f_t.first) << "\t\ttime: " << f_t.second << "   \t\thitnumber: " << function_called_num()[f_t.first] << "\t\taverage time: " << f_t.second/function_called_num()[f_t.first] << std::endl
//总是对不齐？随便调一调对不齐的地方前面空格的个数，说不定就对齐了。

#else
#define ResetClock
#define DisplayClock
#define StartStopWatch
#endif

inline void log() {
    DisplayClock;
    main_log() << std::flush;
    std::ifstream fin("log.dat");
    std::string s;
    while (!fin.eof()) {
        getline(fin, s);
        std::cout << s << std::endl;
    }
    fin.close();
    std::ofstream fileout("log.dat",std::ios::trunc);
    fileout.close();
}


#endif //CODE_LOGGER_H
