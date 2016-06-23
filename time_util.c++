#include <string.h>
#include <iostream>
#include "time_util.hpp"

TimeUtil::TimeUtil(time_t a)
{
   struct tm *tmx;
   char date_str[100];
   time_val = a;
   invalid = 0;

   tmx = localtime(&a);
   strftime(date_str, 100, "%d-%b-%y", tmx);
   time_string = date_str; 
}

TimeUtil::TimeUtil(string& tstr) 
{
   struct tm tm_struct;

   memset(&tm_struct, 0 , sizeof(tm_struct));

   if (!strptime(tstr.c_str(),
             "%d-%b-%y", &tm_struct)) {
        invalid = 1;
        throw "Invalid time str";
   } 
   
   time_val = mktime(&tm_struct);
   if (time_val < 0) {
        invalid = 1;
        throw "Invalid time str";
   }
   time_string = tstr.c_str();
}

TimeUtil::TimeUtil()
{
   struct tm *tmx;
   char date_str[100];
   time_val = 0;
   invalid = 0;

   tmx = localtime(&time_val);
   strftime(date_str, 100, "%d-%b-%y", tmx);
   time_string = date_str; 
}

TimeUtil TimeUtil::operator=( TimeUtil& input)
{
    if (this != &input) {
       time_val = input.time_val;
       time_string = input.time_string;
       invalid = input.invalid ;  
    }
    return *this;
}

string TimeUtil::GetTimeStr() const
{
    return time_string;

}

time_t TimeUtil::GetTimeT() const
{
    return time_val;
}

int TimeUtil::SetTime(time_t a)
{
   struct tm *tmx;
   char date_str[100];
   time_val = a;
   invalid = 0;

   tmx = localtime(&a);
   strftime(date_str, 100, "%d-%b-%y", tmx);
   time_string = date_str; 

   return 0;
}

int TimeUtil::SetTime(string& tstr)
{
   struct tm tm_struct;

   memset(&tm_struct, 0 , sizeof(tm_struct));

   if (!strptime(tstr.c_str(),
             "%d-%b-%y", &tm_struct)) {
        return -1;
   } 
   
   time_val = mktime(&tm_struct);
   if (time_val < 0) {
       return -1;
   }
   time_string = tstr.c_str();
   return 0;
}

time_t TimeUtil::GetNextNthDayT(int n) const
{
    return time_val + 86400 * n;
}

string TimeUtil::GetNextNthDayStr(int n) const
{
    time_t ret_time = time_val + 86400 *n;

    struct tm *tmx;
    char date_str[100];

    tmx = localtime(&ret_time);
    strftime(date_str, 100, "%d-%b-%y", tmx);

    string ret_str(date_str);

    return ret_str;
   
}

