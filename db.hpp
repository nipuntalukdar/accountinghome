#ifndef __DB_HPP__
#define __DB_HPP__
#include <db_cxx.h>
#include <list>
#include <time.h>
#include <string>
using namespace std;

#define ITEM_SIZE 20
#define MAX_INPUT_LINE 255
#define REC_NO_KEY 0
#define CATEGORY_NO_KEY 1
#define CATEGORY_NEXT_ID 2

class Record
{
public:
    char item[ITEM_SIZE];
    time_t date;
    float price;
};

class database
{
public :
    database(string &path, string &dbname, bool isSec = false)throw 
            (DbException, string);
    void show_database_records();
    ~database() ;
    int put_record(Record &r);
    int put_record(list <Record> &records);
    int get_record(list<Record> &records);
    int get_record(list<Record> &records, string &tem);
    int get_record(list<Record> &records, string &item, time_t start_date, 
                    time_t end_date); 
    int get_item(vector<string>& item, list <Record>&list, 
                 double& total_cost,
                 bool show_and_clear = true);

    int rget_item(vector<string>& item_info,
                 list <Record>&list, double& total_cost,
                 bool show_and_clear = true);

    double get_expense(vector<string>& dates, string out_file = "");
    void add_records(string& date);
    int modify_item_name(string& old_name, string& new_name, 
                         string from_date = "", 
                         string to_date = "");
    int modify_item_price(string& item_name, float new_price, 
                         string from_date = "", 
                         string to_date = "");
    double get_total_cost();
    int add_delete_category(string& category_name, bool add=true);
    int show_category(string category_name = "");
    int insert_delete_from_category(string& category_name, string& item_name,
            bool add=true);
    int show_unique_items();
    void show_put_record(Record& r, ostream& os = cout);
    int load_from_file(string& file);
    int load_to_file(string& file_name, string start_date, 
            string end_date); 
    int delete_entries(string& item_name, string& start_date, string& end_date);

private:
    Db *new_db;
    Db *new_db_sec;
    Db *new_db_thi;
    Db *rec_no_db;
    Db *total_cost_db;
    Db *category_db;
    Db *item_category_db;
    DbEnv *env;
    string databaseFile;

    int regex_get_item(vector<string>& matched_items,
         vector <string>& dates, list<Record>& record_list,
        double& total_cost,
        bool show_and_clear);


    double add_to_total_cost(double extra);
};

#endif 
