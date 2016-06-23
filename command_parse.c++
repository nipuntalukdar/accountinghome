#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "command_parse.hpp"
#include "db.hpp"
#include "time_util.hpp"

using namespace std;

const char *valid_commands[] =
{
    "GET [ITEM_NAME] <FROM_DATE> <TO_DATE>",
    "MODIFY ITEM_NAME  [OLD_NAME] [NEW_NAME] <FROM_DATE> <TO_DATE>",
    "MODIFY ITEM_PRICE [ITEM_NAME] [NEW_PRICE] <FROM_DATE> <TO_DATE>",
    "EXPENSE <FROM_DATE> <TO_DATE>", 
    "ADD ITEM <DATE>",
    "ADD CATEGORY [CATEGORY_NAME]",
    "INSERT [ITEM_NAME] TO CATEGORY [CATAEGORY_NAME]",
    "DELETE ITEM [ITEM_NAME] FROM CATEGORY [CATEGORY_NAME]",
    "DELETE ITEM [ITEM_NAME] FROM DATABASE [FROM_DATE] [TO_DATE]",
    "SHOW CATEGORY <CATEGORY_NAME>",
    "DELETE CATEGORY  <CATEGORY_NAME>",
    "SHOW COMMANDS",
    "TOTAL COST ",
    "SHOW UNIQUE_ITEMS",
    "IMPORT FROM [FILENAME]",
    "EXPORT TO [FILENAME] [FROM_DATE] <TO_DATE>",
    "RGET [REGEX_ITEM] <FROM_DATE> <TO_DATE>",
    "EXIT"
};

void db_command_elements::get_command_str()
{
    char *p;
    int ret;
    vector <string> tokens;

    try {
        char command_str[MAX_COMMAND_LEN];
            while (cin.eof() == false){
                cout << "COMMAND>> ";
                cin.getline(command_str,MAX_COMMAND_LEN);
                p = command_str;
                while (*p) {
                    *p = toupper(*p);
                    p++;
                }    

               if (cin.eof() == true)
                   break;
                string command_input = command_str;
                tk->tokenize(command_input, tokens);
                ret = validate_command(command_input, tokens);
                if (0 == ret){
                    if (strcmp(command_str, "EXIT") == 0) 
                        throw "exitnow";

                    this->execute_command(tokens);
                }  else {
                    if (tokens.size() > 0)
                    cout << "    Invalid command\n";
                }
                tokens.clear();
                cin.clear();
                memset(command_str, 0, MAX_COMMAND_LEN);
            }
    } catch (const char *exit_str) {
       tokens.clear();
       cout << "Exiting ... \n";
       return ;
    } catch (...) {
        tokens.clear();
        cout << "Exception was caught \n";
    }
}

void db_command_elements::show_commands()
{
    vector<string> parts;
    show_cmds(commands, parts);
    parts.clear();
}

void db_command_elements::execute_command(vector<string>& tokens)
{
    int index;
#ifdef _DEBUG_ME_
    for (i = 0; i < tokens.size(); i++) {
        cout << tokens[i] << " ";
    }
    cout << endl;
#endif

     if (tokens[0] ==  "GET" || tokens[0] == "RGET") {
         vector <string> getstr;
         list <Record> rl;
         double total_cost = 0;

         if (tokens[1].find_first_of("[ITEM_NAME]") == 0) {
            int pos = tokens[1].find_first_of("=") + 1;
            getstr.push_back(tokens[1].substr(pos));
         }
         if (tokens.size() > 2){
            if (tokens[2].find_first_of("<FROM_DATE>") == 0) {
                int pos = tokens[2].find_first_of("=") + 1;
                getstr.push_back(tokens[2].substr(pos));
            }
         }
         if (tokens.size() > 3) {
            if (tokens[3].find_first_of("<TO_DATE>") == 0) {
                int pos = tokens[3].find_first_of("=") + 1;
                getstr.push_back(tokens[3].substr(pos));
            }
         }
         if (tokens[0] == "GET")
            my_db->get_item(getstr, rl, total_cost);
         else
            my_db->rget_item(getstr, rl, total_cost);

         cout << "Total cost   " << total_cost << endl;
         getstr.clear();

     } else if (tokens[0] == "EXPENSE") {
         vector <string> getstr;
         if (tokens.size() > 1){
            if (tokens[1].find_first_of("<FROM_DATE>") == 0) {
                int pos = tokens[1].find_first_of("=") + 1;
                getstr.push_back(tokens[1].substr(pos));
            }
         }
         if (tokens.size() > 2) {
            if (tokens[2].find_first_of("<TO_DATE>") == 0) {
                int pos = tokens[2].find_first_of("=") + 1;
                getstr.push_back(tokens[2].substr(pos));
            }
         }
         my_db->get_expense(getstr);
         getstr.clear();
     } else if (tokens[0] == "ADD" && tokens[1] == "ITEM") {
        string date_str;
        if (tokens.size() > 2) {
             index = tokens[2].find_first_of("=") + 1;
             date_str = tokens[2].substr(index);
        } else {
            time_t current_time;
            TimeUtil tm(time(&current_time));
            date_str = tm.GetTimeStr();     
        }
        my_db->add_records(date_str);
     } else if ( tokens[0] == "MODIFY"  ) {
        string old_name, new_name;
        string from_date = "", to_date = "";
        int no_item_modified;

        if (tokens[1] == "ITEM_NAME") {
            if (tokens.size() < 4)
                return;

            index = tokens[2].find_first_of("=") + 1;
            old_name = tokens[2].substr(index);
            index = tokens[3].find_first_of("=") + 1;
            new_name = tokens[3].substr(index);

            if (old_name == new_name) {
                cout << "Old name " << old_name << " and " 
                    << "new name " << new_name << " are same. \n";
                return;
            }

            if (tokens.size() > 4) {
                index = tokens[4].find_first_of("=") + 1;
                from_date = tokens[4].substr(index);
            }
            if (tokens.size() > 5){
                index = tokens[5].find_first_of("=") + 1;
                to_date = tokens[5].substr(index);
            }

            no_item_modified = 
                my_db->modify_item_name(old_name, new_name, from_date, 
                    to_date);
            if (no_item_modified < 0) {
                cout << " Some problem in modifying item name \n";
            }
        } else if (tokens[1] == "ITEM_PRICE") {
            if (tokens.size() < 4)
                return;

            string to_date = "", from_date = "";
            index = tokens[2].find_first_of("=") + 1;
            string item_name = tokens[2].substr(index);
            
            index = tokens[3].find_first_of("=") + 1;

            float new_price =  strtod((tokens[3].substr(index)).c_str(), NULL);
            cout << "NEW PRICE " <<  new_price << endl; 
            if (tokens.size() > 4) {
                index = tokens[4].find_first_of("=") + 1;
                from_date = tokens[4].substr(index);
            }
            
            if (tokens.size() > 5) {
                index = tokens[5].find_first_of("=") + 1;
                to_date = tokens[5].substr(index);
            }

            my_db->modify_item_price(item_name, new_price, from_date, to_date);
        }
     } else if (tokens[0] == "TOTAL" && tokens[1] == "COST") {
         my_db->get_total_cost();
     } else if (tokens.size() > 2 && tokens[0] == "ADD" && 
             tokens[1] == "CATEGORY") {
         index = tokens[2].find_first_of("=") + 1;
         string category_name = tokens[2].substr(index);
         my_db->add_delete_category(category_name);
     } else if (tokens[0] == "SHOW") {
        if (tokens.size() > 1 && tokens[1] == "CATEGORY") {
            if (tokens.size() > 2 ) {
                index = tokens[2].find_first_of("=") + 1;
                my_db->show_category(tokens[2].substr(index));    
            } else {
                my_db->show_category();
            }
        } else if (tokens.size() > 1) {
            if (tokens[1] == "UNIQUE_ITEMS")      
                my_db->show_unique_items();
            else if (tokens[1] == "COMMANDS")
               this->show_commands();
        }
     } else if (tokens[0] == "DELETE") {
        if (tokens.size() > 1) {
            if (tokens[1]  == "CATEGORY")  {
               if (tokens.size() > 2) { 
                   index = tokens[2].find_first_of("=") + 1;
                   string cat_name = tokens[2].substr(index);
                   my_db->add_delete_category(cat_name, false);         
               } else {
                   string delete_all("");
                   my_db->add_delete_category(delete_all, false);            
               }
            } else if (tokens[1] == "ITEM") {

                index = tokens[2].find_first_of("=") + 1;
                string item_name = tokens[2].substr(index);

                if (tokens.size() >  4)  {
                    if (tokens[3] == "FROM" && tokens[4] == "DATABASE"){
                        if (tokens.size() == 7) {
                            index = tokens[5].find_first_of("=") + 1;
                            string from_date = tokens[5].substr(index);

                            index = tokens[6].find_first_of("=") + 1;
                            string to_date = tokens[6].substr(index);

                            my_db->delete_entries(item_name, from_date, 
                                    to_date);
                        }
                    } else if (tokens[3] == "FROM" && tokens[4] == "CATEGORY"){
                        if (tokens.size() == 6) { 
                            index = tokens[5].find_first_of("=") + 1; 
                            string category_name = tokens[5].substr(index);
                    
                            my_db->insert_delete_from_category(category_name, 
                            item_name , false);
                        }
                    }
                }
            }
        }

     } else if (tokens[0] == "INSERT" && tokens.size() > 4) {

        index = tokens[1].find_first_of("=") + 1;
        string item = tokens[1].substr(index);
        index = tokens[4].find_first_of("=") + 1;
        string category = tokens[4].substr(index);        

        my_db->insert_delete_from_category(category, item, true);

     }else  if (tokens[0] == "IMPORT" && tokens.size() == 3) {

        index = tokens[2].find_first_of("=") + 1;
        string file_name = tokens[2].substr(index);
        my_db->load_from_file(file_name);

     } else if (tokens[0] == "EXPORT" && tokens.size() > 2) {
        index = tokens[2].find_first_of("=") + 1;        
        string file_name = tokens[2].substr(index);
        string to_date = "", from_date = "";
        
        if (tokens.size() > 3) {
            index = tokens[3].find_first_of("=") + 1;
            from_date = tokens[3].substr(index); 
        }

        if (tokens.size() > 4) {
            index = tokens[4].find_first_of("=") + 1;
            to_date = tokens[4].substr(index); 
        }

        my_db->load_to_file(file_name, from_date, to_date);

     }
    
}

void db_command_elements::show_cmds(vector<element>& cmds, 
        vector<string>& parts)
{
    unsigned int i;
    for ( i = 0; i < cmds.size(); i++) {
        parts.push_back(cmds[i].get_command());
        if (cmds[i].get_next_commands().size() == 0) {
            unsigned int j = 0;
            while (j < parts.size()) {
                cout << parts[j++] << "  ";
            }
            cout << endl;
        } else {
           show_cmds(cmds[i].get_next_commands(), parts);
        }
        parts.pop_back();
    }
}


int db_command_elements::validate_command(string& input_command, 
        vector<string>& tokens)
{
    vector <element>* com_el;
    vector <element>::iterator it;
    int token_num = 0, total_tokens, command_part_found;
    if (input_command.size() == 0 || tokens.size() == 0)
        return -1;

    
    com_el = &commands;
    it = com_el->begin();
    total_tokens = tokens.size();

    while (token_num < total_tokens){
        command_part_found = 0;
        while (it != com_el->end()) {
            if (it->get_command() == tokens[token_num]){
                com_el = it->get_pointer();
                it = com_el->begin();
                command_part_found = 1;
                break;
            } else if (it->get_command().find_first_of("[") == 0 &&
                    it->get_command().find_last_of("]")== 
                    (it->get_command().size() - 1) ){
                string mod_command = it->get_command() + "=" +
                        tokens[token_num] ;
                tokens[token_num] = mod_command;   
                command_part_found = 1;
            } else if  (it->get_command().find_first_of("<") == 0 &&
                    it->get_command().find_last_of(">")== 
                    (it->get_command().size() - 1) ) {
                
                string mod_command = it->get_command() + "=" +
                        tokens[token_num] ;
                tokens[token_num] = mod_command;   
                command_part_found = 1;
            }
            if (command_part_found) {
                com_el = it->get_pointer();
                it = com_el->begin();
                break;
            }
            it++;
        }
        if (!command_part_found){
            return -1;
        }
        token_num++;
    }
    if (com_el->size() > 0){
        /* 
         * Everything matched, but we may be missing some elements 
         */
        vector <element>::iterator it_end = 
            com_el->end();
        string cur_command;

        it = com_el->begin();
        while (it != it_end) {
           cur_command = it->get_command();
           if (cur_command.find_first_of("<") == 0 &&(
                cur_command.find_last_of(">") == (cur_command.size() - 1))){
               it++;
               continue;
           } else {
               return -1;
           }           
        }
    }

    return 0;
}

db_command_elements::db_command_elements(const char *valid_commands[], 
        int total_commands,
        string& database_path,
        string& database_name
        )
{
    int num_commands = 0, done_this_level;
    unsigned int command_el_no;
    vector<string> tokens;
    tk = new tokenizer();
    vector<element>::iterator it;
    vector<element> *com_el;
    com_el = &commands;

    while (num_commands < total_commands ) {
        string command_string = string(valid_commands[num_commands++]);
        tk->tokenize(command_string, tokens); 
        
        
        if (tokens.size() == 0)
            continue;
        com_el = &commands;
        it = com_el->begin();

        for (command_el_no = 0; command_el_no < tokens.size();command_el_no++){
            done_this_level = 0;
            while (1){
                if (it == com_el->end()){
                    done_this_level = 1;
                    it = com_el->insert(it, element(tokens[command_el_no]));
                } 
                else if (it->get_command() == tokens[command_el_no]){
                    done_this_level = 1;
                }
                if (done_this_level) {
                    com_el = it->get_pointer();
                    it = com_el->begin();
                    break;
                }
                it++;
            }
        }
        tokens.clear();
    }

    try {
        my_db = new database(database_path, database_name);
    } catch (DbException& e) {
        cout << "Error in creating database " << e.what() << endl;
        throw "error";
    } catch (...) {
        cout << "Error creating database \n";
        throw "error";
    }
}

int main(int argc, char *argv[])
{
    string  database_name = "accounts";
    string  database_dir = "";
    if (argc > 1) {
        cout << "Using " << argv[1] << "/accounting directory for database" << endl;
        struct stat sb;
        if (stat(argv[1], &sb) != 0 || !S_ISDIR(sb.st_mode)) {
            cerr << argv[1] << " either not directory or directory doesn't exists" << endl;
            exit(1);
        }
        string temp = argv[1];
        database_dir = temp + "/accounting";
    } else {
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        database_dir = homedir;
        database_dir += "/accounting";
        cout << "Using database in directory " <<  database_dir << endl;
    }
    // check if the directory for database exists 
    struct stat sb; 
    if (stat(database_dir.c_str(), &sb) == 0) {
        if (!S_ISDIR(sb.st_mode)) {
            cerr << database_dir << " is not a directory" << endl;
            exit(1);
        }
    } else {
        // try to create the directory
        if (mkdir(database_dir.c_str(), S_IRWXU) != 0){
            cerr << "Could not create database directory " << database_dir << endl;
            exit(1);
        }

    }
    db_command_elements x(valid_commands, sizeof(valid_commands) / sizeof(void*),
                 database_dir, database_name );
    x.get_command_str();

    return 0;
}
