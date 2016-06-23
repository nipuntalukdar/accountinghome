#include <iostream>
#include <vector>
#include <string>
#include "string_tokenize.hpp"
using namespace std;

int tokenizer::tokenize(string& y, vector<string>& tokens)
{
    const char *white_chars = delims.c_str();
    if (y.length() == 0){
        return -1;      
    }

    size_t m = 0,l;
    do {
        l = y.find_first_of(white_chars, m);
        if ( l == string::npos){
            if (y.substr(m).length() > 0)
                tokens.push_back(y.substr(m));
            break;
        } else if (l > m){
            tokens.push_back(y.substr(m, l -m));
        } 

        m = y.find_first_not_of(white_chars, l);
        if (m == string::npos)
            break;
        l = y.find_first_of(white_chars, m); 
        
        if ( l == string::npos) {
           tokens.push_back(y.substr(m));
           break;
        }  
        tokens.push_back(y.substr(m, l - m));
        m = y.find_first_not_of(white_chars, l);
        if (m == string::npos)
            break;
    } while (1);

    return 0;
}


