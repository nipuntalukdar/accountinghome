#ifndef __TIME_UTIL__
#define __TIME_UTIL__
#include <time.h>
#include <sys/types.h>
#include <iostream>
#include <string>
using namespace std;

class TimeUtil
{
public:
    TimeUtil();
    TimeUtil(time_t a);
    TimeUtil(string& time_str);
    time_t GetTimeT() const;    
    string GetTimeStr() const;
    time_t GetNextNthDayT(int n) const;
    string GetNextNthDayStr(int n) const;
    int SetTime(time_t t);
    int SetTime(string& new_time_str);
    TimeUtil operator = (TimeUtil& tm);

 private:
    string time_string;
    time_t time_val;
    int invalid;
};

#endif 
