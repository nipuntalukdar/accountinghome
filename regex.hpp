#include <stdio.h>
#include <string>
using namespace std;

class RegEx
{
    public:
        RegEx(string current_expr = "") throw (const char *);
        RegEx(const char *str) throw (const char *);
        void SetExStr(string new_str="") throw (const char *);
        void SetExStr(const char *str) throw (const char *);
        int Match(string to_match) throw (const char *);
        ~RegEx();
    
    private:
        void *re;
        int state;
        string current_expr;      
};



