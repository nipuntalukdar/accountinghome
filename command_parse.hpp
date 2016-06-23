#ifndef  __COMMAND_PARSE_HPP__
#define  __COMMAND_PARSE_HPP__ 
#include <vector>
#include "string_tokenize.hpp"
#include <string>
#include "db.hpp"


#define MAX_COMMAND_LEN 255

class element
{
    public:
    element(string x) { command_el = x; }
    string& get_command() { return command_el ; }
    vector<element> & get_next_commands() 
    {
        return next_command_els;
    }
    element& get_current_command() 
    {
        return *this;
    }

    vector<element> * get_pointer()
    {
        return &next_command_els;
    }
    private:
    string command_el;
    vector <element> next_command_els;
};

class db_command_elements
{
public:
    db_command_elements(const char *command_array[], int size_of_array,
            string& database_dir, string& database_name);
    ~db_command_elements() 
    {
       delete tk;
       delete my_db;
    }
    void show_commands();
    void get_command_str();
private:
    vector<element> commands;
    tokenizer *tk;
    void show_cmds(vector<element>& cmds, 
                    vector<string>& parts);
    int validate_command(string& command_str, vector<string>& tokens);
    void execute_command(vector<string>& tokens);
    database *my_db;
    
};
#endif
