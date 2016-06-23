#include "regex.hpp"
#include <pcre.h>

#define MAX_MATCHES 30
#define GOOD_STATE 0
#define BAD_STATE 1

const char *excp_str = "Regex object is in bad state";

RegEx::RegEx(string regex) throw (const char *)
{
    state = GOOD_STATE;
    re = 0;
    SetExStr(regex); 
}

RegEx::RegEx(const char *str) throw (const char *)
{
    state = GOOD_STATE;
    re = 0;
    
    if (!str) {
        state = BAD_STATE;
        throw excp_str;
    }   

    SetExStr(string(str)); 
}

void RegEx::SetExStr(const char *str) throw (const char *)
{
    const char *errptr;
    int erroroffset;

    if (!str) 
        return;

    current_expr = str;

    if (re)
        pcre_free(re);
    
    re = pcre_compile(current_expr.c_str(),
            0, &errptr,&erroroffset, 0);

    state = GOOD_STATE;

    if (!re) {
        state = BAD_STATE;
        throw excp_str;
    }
}

void RegEx::SetExStr(string regex) throw (const char *)
{
    const char *errptr;
    int erroroffset;
    current_expr = "[.\n]*";

    if (re)
        pcre_free(re);

    if (regex.size() > 0)
        current_expr = regex;
    else
        return;
    
    re = pcre_compile(current_expr.c_str(),
            0, &errptr,&erroroffset, 0);

    state = GOOD_STATE;

    if (!re) {
        state = BAD_STATE;
        throw excp_str;
    }
}

int RegEx::Match(string to_match) throw (const char *)
{
    int rc;
    int offsets[MAX_MATCHES];

    if (to_match.size() == 0)
        return -1;

    if (state == BAD_STATE)
        throw excp_str;

    rc = pcre_exec((pcre *)re, NULL, to_match.c_str(), 
            to_match.size() + 1, 0,0,
            offsets, MAX_MATCHES);

    if (rc < 0 ) {
        return -1;
    }   

    return offsets[0];
    
}

RegEx::~RegEx()
{
    pcre_free(re);
}

