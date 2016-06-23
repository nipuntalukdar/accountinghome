#ifndef __STRING_TOKENIZER__
#define __STRING_TOKENIZER__
#include <string>
#include <vector>
using namespace std;

class tokenizer 
{   
public :
    tokenizer(const char *str="\n\t\b\r \0") {
        delims = string(str);
    }
    int tokenize(string& text, vector<string>& tokens);
private :
    string delims;
};
#endif
