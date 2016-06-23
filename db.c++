#include <errno.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <list>
#include <algorithm>
#include <memory>
#include <exception>
#include <vector>
#include <db.h>
#include <db_cxx.h>
#include <fstream>
#include "string_tokenize.hpp"
#include "db.hpp"
#include "regex.hpp"
#include "time_util.hpp"

using namespace std;


class  db_entry
{
public:
    char item[ITEM_SIZE];
    float price;
};


bool recordComparator(Record& r1, Record& r2)
{
    string item1(r1.item);
    string item2(r2.item);
   
    if (r1.date != r2.date) 
           return r1.date < r2.date;

    if (item1 != item2)
        return item1 < item2;

    return r1.price < r2.price;

}

bool reverse_recordComparator(Record& r1, Record& r2)
{
    string item1(r1.item);
    string item2(r2.item);
   
    if (r1.date != r2.date) 
           return r1.date > r2.date;

    if (item1 != item2)
        return item1 > item2;

    return r1.price > r2.price;

}

static int name_extractor(
        Db *sec,
        const Dbt *pKey,
        const Dbt *pData,
        Dbt *secKey
        )
{
    Record *en = (Record *)pData->get_data();
    secKey->set_data(en->item);
    secKey->set_size(strlen(en->item) + 1);
    return 0;    
}

static int date_compare(
        Db *db,
        const Dbt *key1,
        const Dbt *key2
        )
{
    time_t t1 = *((time_t *)key1->get_data()),
                 t2 =   *((time_t *)key2->get_data());
    return (int )(t1 - t2);

}

static int int_key_compare(
        Db *db,
        const Dbt *key1,
        const Dbt *key2
        )
{
    int  a1 = *((int *)key1->get_data()),
                 a2 =   *((int *)key2->get_data());
    return (a1 - a2);
}

static int date_extractor(
        Db *sec,
        const Dbt *pKey,
        const Dbt *pData,
        Dbt *secKey
        )
{
    Record *en = (Record *)pData->get_data();
    secKey->set_data(&en->date);
    secKey->set_size(sizeof(en->date));
    return 0;    
}

database::database(string &path, string &db_name , 
             bool isSec)throw (DbException 
                    , string)
{
    int ret;
    unsigned int env_flags = DB_CREATE | DB_INIT_MPOOL ;
    databaseFile = path;
    env = new DbEnv(0);
    env->open(databaseFile.c_str(),env_flags, 0); 

    new_db = new Db(env,0);
    new_db_sec = new Db(env, 0);
    new_db_thi = new Db(env, 0);
    rec_no_db = new Db(env, 0);
    total_cost_db = new Db(env, 0);
    category_db = new Db(env, 0);
    item_category_db = new Db(env, 0);

    new_db_sec->set_flags(DB_DUPSORT);
    new_db_thi->set_flags(DB_DUPSORT);
    item_category_db->set_flags(DB_DUPSORT);
    item_category_db->set_bt_compare(int_key_compare);
    new_db_thi->set_bt_compare(date_compare);

    new_db->open(NULL,db_name.c_str(), "PRIMARY", DB_BTREE, DB_CREATE, 0);

    new_db_sec->open(NULL,db_name.c_str(), "SECONDARY_1", DB_BTREE, DB_CREATE, 0);

    new_db_thi->open(NULL,db_name.c_str(), "SECONDARY_2", DB_BTREE, DB_CREATE, 0);

    category_db->open(NULL, db_name.c_str(), "ITEM_CATEGORY", DB_BTREE, 
            DB_CREATE, 0);

    item_category_db->open(NULL, db_name.c_str(), "MAP_ITEM_CATEGORY", DB_BTREE, 
            DB_CREATE, 0);




    new_db->associate(NULL, new_db_sec, name_extractor, 0);
    new_db->associate(NULL, new_db_thi, date_extractor, 0);
    
    try {
        ret  = total_cost_db->open(NULL, db_name.c_str(), "TOTAL_EXPENSE", 
            DB_BTREE, DB_CREATE, 0);
        if (ret == 0) {
            Dbt dKey, dData;
            int key = 1;
            dKey.set_size(sizeof(key));
            dKey.set_data(&key);
            if (total_cost_db->get(NULL, &dKey, &dData, 0) == DB_NOTFOUND){
               Dbt dData;
               double total_cost = 0.0;
               dData.set_data(&total_cost);
               dData.set_size(sizeof(total_cost));
               total_cost_db->put(NULL, &dKey, &dData, 0);
            }
        }
    } catch (DbException& e) {
        cout << "Caught exception " << e.what() << endl;
        exit(1);
    }
    try {
        ret = rec_no_db->open(NULL, db_name.c_str(), "REC_NO_DB", DB_BTREE, 
                DB_CREATE, 0);
         if (ret == 0) {
            Dbt dKey;
            Dbt dData;
            int key = REC_NO_KEY;
            dKey.set_data(&key);
            dKey.set_size(sizeof(key));

            if (rec_no_db->get(NULL, &dKey, &dData, 0) == DB_NOTFOUND){
               int next_rec_no = 1;
               dData.set_data(&next_rec_no);
               dData.set_size(sizeof(next_rec_no));
               rec_no_db->put(NULL, &dKey, &dData, 0);
            }

            key = CATEGORY_NO_KEY;
            if (rec_no_db->get(NULL, &dKey, &dData, 0) == DB_NOTFOUND){
               int category_no = 0;
               dData.set_data(&category_no);
               dData.set_size(sizeof(category_no));
               rec_no_db->put(NULL, &dKey, &dData, 0);
            }

            key = CATEGORY_NEXT_ID;
            if (rec_no_db->get(NULL, &dKey, &dData, 0) == DB_NOTFOUND){
               int category_next_id = 1;
               dData.set_data(&category_next_id);
               dData.set_size(sizeof(category_next_id));
               rec_no_db->put(NULL, &dKey, &dData, 0);
            }
        }
    } catch (DbException& e) {
        cout << "Caught exception " << e.what() << endl;
        exit(1);
    }
}

database::~database()
{
    new_db_thi->close(0);
    new_db_sec->close(0);
    rec_no_db->close(0);
    total_cost_db->close(0);
    category_db->close(0);
    new_db->close(0);
    item_category_db->close(0);
    env->close(0);

    delete new_db_sec;
    delete new_db_thi;
    delete total_cost_db;
    delete rec_no_db;
    delete category_db;
    delete item_category_db;
    delete new_db;
    delete env;

}


void database::show_put_record(Record& r, ostream& os)
{
    char date_str[100];
    strftime(date_str, 100, "%d-%b-%y", localtime(&r.date));
    os.setf(ios::right);
    os << setw(20) << r.item  << " " << setw(10) << fixed 
        << setprecision(2) << r.price << " " 
        << setw(10) << date_str << endl;
    os.unsetf(ios::right);
}

//
//load to file will load the expense report item wise from
//from_date to to_date
// 

int database::load_to_file(string& file_name,
        string from_date,
        string to_date)

{
    vector <string> dates;

    if (file_name.size() == 0 || from_date.size() == 0)
        return -1;

    
    dates.push_back(from_date);

    if (to_date.size() > 0)
        dates.push_back(to_date);

    if (get_expense(dates, file_name) < 0)
        return -1;
    return 0;    
}

int database::load_from_file(string& file)
{
    ifstream in;
    vector <string> tokens;
    list <Record> records;
    Record r;
    int copy_size;
    tokenizer tk;
    string input;
    TimeUtil tm;

    try {
        in.open(file.c_str());
        char input_str[MAX_INPUT_LINE];
        if (in.is_open() == false){
            cout << "File could not be opened \n";
            return -1;
        }

        while (in.eof() == false) {
            in.getline(input_str, MAX_INPUT_LINE);
            input = input_str;
            tk.tokenize(input, tokens);
            memset(&r, 0 ,ITEM_SIZE);

            if (tokens.size() == 3){
                do  {
                    if (tokens[0].size() == 0 || 0 
                            || tokens[1].size() == 0 ||tokens[2].size() == 0)
                        break;

                        copy_size = tokens[0].size();
                   
                    if (copy_size > ITEM_SIZE)
                            copy_size = ITEM_SIZE;
                   
                    memcpy(r.item, tokens[0].c_str(), copy_size); 
                    r.price = strtod(tokens[1].c_str(), NULL); 
                    tm.SetTime(tokens[2]);

                    r.date  = tm.GetTimeT();

                    show_put_record(r);
                    records.push_back(r);
                } while (0);
            }
            tokens.clear();

            if (in.eof() ==  true)
                break;
        }
       
        if (records.size() > 0) {
            put_record(records);
            records.clear();
        } 

    } catch (DbException& e) {
        cout << "Exception: " << e.what() << endl;
    } catch (...) {
        cout << " There was some problem \n";
    }
    if (in.is_open())
        in.close();

    return 0;
}

//
//Returns the number of unique items 
//

int database::show_unique_items()
{
    Dbt dKey, dData;
    int ret_val;
    Dbc *item_cursor = NULL;
    
    new_db_sec->cursor(NULL, &item_cursor, 0);
    
    ret_val = item_cursor->get(&dKey, &dData, DB_NEXT);

    while (ret_val == 0) {    
        char *item_name = (char *)dData.get_data();
        ret_val++;
        cout << item_name << endl;
        ret_val = item_cursor->get(&dKey, &dData, DB_NEXT_NODUP);
    }   
    
    if (item_cursor)
        item_cursor->close();
    return ret_val;
}

int database::get_record(list<Record>& lst)
{
    Dbc *cursor;
    Dbt key, data;
    Record rec;
    Record *entry;

    system("clear");

    try {
        new_db->cursor(NULL, &cursor, 0);
        while (cursor->get(&key, &data,DB_NEXT) == 0) {
            entry = (Record *)data.get_data();
            memcpy(rec.item, entry->item , ITEM_SIZE); 
            rec.date = entry->date;
            rec.price = entry->price;
            lst.push_back(rec);        
        }
        cursor->close();
    } catch (DbException& e) {
        cerr << " Error in getting data " << e.what() << endl;
        return -1;
    } catch (...) {
        cerr << " Error in getting data \n";
         return -1;
    }
    return 0;
}

double database::get_expense(vector<string>& item_dates, string out_file)
{
    /*
     * Here the first item should be item_name, and second and third items ate
     * optional to and from date in the format.eg. 20-nov-07
     */
    Dbt fromDateKey;
    Dbt primData;
    Dbc *cursor_date;
    double total_cost = 0.0;
    double retval = -1.0;
    list <Record> record_list;
    list <Record> ::iterator it;
    ofstream out;

    if (item_dates.size() == 0)
        return -1;
    
    if (out_file.size() > 0){
        out.open(out_file.c_str());
        if (!out)
            return -1;        
    } 

    try {
        new_db_thi->cursor(NULL,&cursor_date, 0);
        TimeUtil tm1, tm2 ;
        time_t start, end;
        TimeUtil start_tm(item_dates[0]);
        tm1 = start_tm;
        start = tm1.GetTimeT();

        if (item_dates.size() > 1) {
            TimeUtil end_tm = TimeUtil(item_dates[1]);
            tm2 = end_tm;
            end = tm2.GetTimeT();
        } else {
            end = 0;
        }
        
        if (end) {
            if (end < start ) {
                time_t temp = end;
                end = start;
                start = temp;
                end = tm1.GetNextNthDayT(1);
            } else {
                end = tm2.GetNextNthDayT(1);
            }
        }

        fromDateKey.set_data(&start);
        fromDateKey.set_size(sizeof(start));

        if (cursor_date->get(&fromDateKey, &primData, DB_SET_RANGE) != 0)
            throw "record not found";
        do{
            Record *r = (Record *)primData.get_data();
            if (end != 0 && r->date >= end)
                break;
            record_list.push_back(*r);
            total_cost += r->price;
        } while (cursor_date->get(&fromDateKey, &primData, DB_NEXT) == 0);
        
        if (record_list.size() > 0) {

            record_list.sort(recordComparator);
            it = record_list.begin(); 

            if (out.is_open() == true){
                while (it != record_list.end()) {
                    show_put_record(*it, out);
                    it++; 
                } 
                out.close();
            } else {
                while (it != record_list.end()) {
                    show_put_record(*it);
                    it++; 
                } 
            }     
            record_list.clear();
        }

        retval = total_cost;
        cout << "\n\n TOTAL COST " << total_cost << endl;

    } catch (DbException& e) {          
        cout << "Problem " << e.what() << endl;
        retval = -1.0;
    } catch (...) {
        cout << "Some problem in getting item info \n";
        retval = -1.0;
    }

    if (cursor_date != NULL)
        cursor_date->close();
    
    return retval;
}

int database::modify_item_price(string& item_name, float new_price,
                               string from_date, string to_date)
{
    int item_price_changed = 0, ret_val;
    Dbc *name_cursor = NULL, *date_cursor = NULL;
    Dbt dKey, dData, dReplace_Data, pKey;
    Dbt dKeyName;
    Record replace_record;

    if ( item_name.size() == 0 ||
         (from_date == "" && to_date != ""))
        return -1;

    memset(replace_record.item, 0 , ITEM_SIZE);
    memcpy(replace_record.item, item_name.c_str(),
                    (item_name.size() >= ITEM_SIZE) ?
                    ITEM_SIZE -1 : item_name.size());
    replace_record.price = new_price;

    try {
        double change_in_total_cost = 0;

        new_db_sec->cursor(NULL, &name_cursor, 0);
        dReplace_Data.set_data(&replace_record);
        dReplace_Data.set_size(sizeof(replace_record));
        dKey.set_data((void *)item_name.c_str());
        dKey.set_size(item_name.size() + 1);

        if (from_date == "") {
            ret_val = name_cursor->pget(&dKey, &pKey, &dData, DB_SET);
            if (ret_val == DB_NOTFOUND) {
                cout << " Item " << item_name << " not found \n";
                name_cursor->close();
                return 0;
            }

            do {
                Record *r = (Record *) dData.get_data();

                if (new_price != r->price) {
                    replace_record.date = r->date;
                    // delete the corresponding primary record , and recreate the
                    // record for the key with the modified value

                    change_in_total_cost += (new_price - r->price); 
                    new_db->del(NULL, &pKey , 0);
                    new_db->put(NULL, &pKey, &dReplace_Data, 0);
                }
            } while (0 == name_cursor->pget(&dKey, &pKey, &dData, DB_NEXT_DUP));

        } else {
            TimeUtil tm_to;
            time_t from_t, to_t, temp_t;
            Dbt timeData;
            Record *r;

            new_db_thi->cursor(NULL, &date_cursor, 0);
            new_db_sec->cursor(NULL, &name_cursor, 0);

            if (to_date.size() == 0){
                // to_t should point to the last date 
                Dbt keyAll;
                Dbt dataAll;
                ret_val = date_cursor->get(&keyAll,&dataAll, DB_LAST);
                to_t = *((time_t *) keyAll.get_data());
            } else {
               TimeUtil to_time(to_date); 
               tm_to = to_time;
               to_t = tm_to.GetTimeT();
            }

            TimeUtil tm_from(from_date);
            from_t = tm_from.GetTimeT(); 

            if (to_t < from_t) {
                temp_t = from_t;
                from_t = to_t;
                to_t = temp_t;
            } 


            // position the name cursor at the beginning of item_name
            dKeyName.set_data((void *)item_name.c_str());
            dKeyName.set_size(item_name.size() + 1);

            if (name_cursor->pget(&dKeyName, &pKey, &dData, DB_SET)!= 0) {
                throw ": Item not found";
            }

            // Positon the date cursor to "from date";
            dKey.set_data(&from_t);
            dKey.set_size(sizeof(from_t));

            ret_val = date_cursor->get(&dKey, &timeData, 
                    DB_SET_RANGE); 

            if (ret_val != 0)
                throw ": Item not found within given dates";
    
            from_t = *((time_t *)dKey.get_data()); 
            if (from_t  > to_t)
                throw ": Item not found within given dates ";  

            replace_record.price = new_price;

            do {
                r = (Record *)dData.get_data();
                temp_t = r->date;
                if (r->date >= from_t  && r->date <= to_t && 
                        r->price != new_price){
                   
                    replace_record.date = r->date;
                    change_in_total_cost += (new_price - r->price);
                    new_db->del(NULL, &pKey, 0);
                    new_db->put(NULL, &pKey, &dReplace_Data, 0);
                }
            } while (name_cursor->pget(&dKeyName, &pKey, &dData, 
                        DB_NEXT_DUP) == 0);
        }

        if (change_in_total_cost != 0)
            add_to_total_cost(change_in_total_cost);

    } catch (DbException& e) {
        cout << "Problem " << e.what() << endl;
        item_price_changed = -1;
    } catch (const char *str) {
        cout << "Caught " << str << endl;
        item_price_changed = -1;
    } catch ( ... ) {
        cout << "Problem in changing item name \n";    
        item_price_changed = -1;
    }
    if (name_cursor)
        name_cursor->close();
    if (date_cursor)
        date_cursor->close();

    return item_price_changed ;
}

int database::modify_item_name(string& old_name, string& new_name,
                               string from_date, string to_date)
{
    int item_name_changed = 0, ret_val;
    Dbc *name_cursor = NULL, *date_cursor = NULL;
    Dbt dKey, dData, dReplace_Data, pKey;
    Dbt dKeyName;
    Record replace_record;

    if ( old_name.size() == 0 || new_name.size() == 0 ||
         (from_date == "" && to_date != ""))
        return -1;

    cout << "Changing name " << old_name << " to " 
        << new_name << endl;
    try {

        new_db_sec->cursor(NULL, &name_cursor, 0);
        memcpy(replace_record.item, new_name.c_str(), ITEM_SIZE);
        dReplace_Data.set_data(&replace_record);
        dReplace_Data.set_size(sizeof(replace_record));
        dKey.set_data((void *)old_name.c_str());
        dKey.set_size(old_name.size() + 1);

        if (from_date == "") {
            ret_val = name_cursor->pget(&dKey, &pKey, &dData, DB_SET);
            if (ret_val == DB_NOTFOUND) {
                cout << " Item " << old_name << " not found \n";
                name_cursor->close();
                return 0;
            }

            do {
                Record *r = (Record *) dData.get_data();
                replace_record.price = r->price;
                replace_record.date = r->date;

                // delete the corresponding primary record , and recreate the
                // record for the key with the modified value
                
                new_db->del(NULL, &pKey , 0);
                new_db->put(NULL, &pKey, &dReplace_Data, 0);
            } while (0 == name_cursor->pget(&dKey, &pKey, &dData, DB_SET));

        } else {
            TimeUtil tm_to;
            time_t from_t, to_t, temp_t;
            Dbt timeData;
            Record *r;

            new_db_thi->cursor(NULL, &date_cursor, 0);
            new_db_sec->cursor(NULL, &name_cursor, 0);

            if (to_date.size() == 0){
                // to_t should point to the last date 
                Dbt keyAll;
                Dbt dataAll;
                ret_val = date_cursor->get(&keyAll,&dataAll, DB_LAST);
                to_t = *((time_t *) keyAll.get_data());
            } else {
               TimeUtil to_time(to_date); 
               tm_to = to_time;
               to_t = tm_to.GetTimeT();
            }

            TimeUtil tm_from(from_date);
            from_t = tm_from.GetTimeT(); 

            if (to_t < from_t) {
                temp_t = from_t;
                from_t = to_t;
                to_t = temp_t;
            } 


            // position the name cursor at the beginning of item_name
            dKeyName.set_data((void *)old_name.c_str());
            dKeyName.set_size(old_name.size() + 1);

            if (name_cursor->pget(&dKeyName, &pKey, &dData, DB_SET)!= 0) {
                throw ": Item not found";
            }

            // Positon the date cursor to "from date";
            dKey.set_data(&from_t);
            dKey.set_size(sizeof(from_t));

            ret_val = date_cursor->get(&dKey, &timeData, 
                    DB_SET_RANGE); 

            if (ret_val != 0)
                throw ": Item not found within given dates";
    
            from_t = *((time_t *)dKey.get_data()); 
            if (from_t  > to_t)
                throw ": Item not found within given dates ";  
            do {
                r = (Record *)dData.get_data();
                temp_t = r->date;
                if (r->date >= from_t  && r->date <= to_t){
                    replace_record.price = r->price;
                    replace_record.date = r->date;
                    new_db->del(NULL, &pKey, 0);
                    new_db->put(NULL, &pKey, &dReplace_Data, 0);
                }
            } while (name_cursor->pget(&dKeyName, &pKey, &dData, 
                        DB_NEXT_DUP) == 0);
        }

    } catch (DbException& e) {
        cout << "Problem " << e.what() << endl;
        item_name_changed = -1;
    } catch (const char *str) {
        cout << "Caught " << str << endl;
        item_name_changed = -1;
    } catch ( ... ) {
        cout << "Problem in changing item name \n";    
        item_name_changed = -1;
    }
    if (name_cursor)
        name_cursor->close();
    if (date_cursor)
        date_cursor->close();

    return item_name_changed ;
}

int database::regex_get_item(vector<string>& matched_items,
         vector <string>& dates, list<Record>& record_list,
        double& total_cost,
        bool show_and_clear)
{
    /*
     * Here the first item should be item_name, and second and third items ate
     * optional to and from date in the format.eg. 20-nov-07
     */
    Dbt itemKey;
    Dbt fromDateKey;
    Dbt toDateKey;
    Dbt primData;
    int retval = 0;

    try {
        int i = 0, size_items = matched_items.size();
        vector <string> item_info;
        // Get the info for all the matched elements one by one
        item_info.push_back("");

        if (dates.size() > 0)
            item_info.insert(item_info.end(), dates.begin(), dates.end());

        while (i < size_items) {
            item_info[0] = matched_items[i++];
            get_item(item_info, record_list, total_cost, show_and_clear );    
        }
        

    } catch (DbException& e) {          
        cout << "Problem " << e.what() << endl;
        retval = -1;
    } catch (...) {
        cout << "Some problem in getting item info \n";
        retval = -1;
    }
    
    return retval;
   
}

int database::rget_item(vector<string>& item_info,
                      list<Record>& record_list,
                      double& total_cost,
                      bool show_and_clear)
{
    /*
     * Here the first item should be item_name, and second and third items ate
     * optional to and from date in the format.eg. 20-nov-07
     */
    Dbt dKey;
    Dbt dData;
    int retval = 0;
    vector <string> matched_items, dates;
    Dbc *cursor_name;

    if (item_info.size() == 0)
        return -1;

    total_cost = 0;

    try {
        new_db_sec->cursor(NULL,&cursor_name, 0);
        RegEx rgx(item_info[0]);
        retval = cursor_name->get(&dKey, &dData, DB_NEXT);
        
        while (retval == 0) {
           char *item_name = (char *)dData.get_data();
           if (rgx.Match(item_name) >= 0){
               matched_items.push_back(string(item_name));
               sort(matched_items.begin(), matched_items.end()); 
           }
           retval = cursor_name->get(&dKey, &dData, DB_NEXT_NODUP);
        }
    } catch (DbException& e) {          
        cout << "Problem " << e.what() << endl;
        retval = -1;
    } catch (const char *str) {
        cout << "Problem " << str << endl;
        retval = -1;
    } catch (...) {
        cout << "Some problem in getting item info \n";
        retval = -1;
    }

    
    if (cursor_name!= NULL)
        cursor_name->close();
   
    if (retval < 0 && retval != DB_NOTFOUND)
       return -1;
    
    if (matched_items.size() == 0)     
        return 0;

    vector <string> :: iterator it = item_info.begin();
    it++;
    
    dates.insert(dates.begin(), it , item_info.end());

    return regex_get_item(matched_items, dates, record_list, total_cost,
            show_and_clear);
    
}

int database::get_item(vector<string>& item_info,
                      list<Record>& record_list,
                      double& total_cost,
                      bool show_and_clear)
{
    /*
     * Here the first item should be item_name, and second and third items ate
     * optional to and from date in the format.eg. 20-nov-07
     */
    Dbt itemKey;
    Dbt fromDateKey;
    Dbt toDateKey;
    Dbt primData;
    int retval = 0;

    Dbc *cursor_name;
    Dbc *cursor_date;

    if (item_info.size() == 0)
        return -1;

    try {
        new_db_sec->cursor(NULL,&cursor_name, 0);
        new_db_thi->cursor(NULL,&cursor_date, 0);

        string item_name = item_info[0];
        if (item_name.size() == 0)
            throw "problem";
        itemKey.set_data((void *)item_name.c_str());
        itemKey.set_size(item_name.size() + 1);

        if (cursor_name->get(&itemKey, &primData, DB_SET) != 0)
            throw "record not found";
        if (item_info.size() == 1) {
            do {
                Record *r;
                r = (Record *) primData.get_data();
                record_list.push_back(*r);
                total_cost += r->price;
            } while (cursor_name->get(&itemKey, &primData, DB_NEXT_DUP) == 0);
        } else {
            try {
                // Join wont't work. Join looks for some kind of exact match
                // while we look for kind of range match
                TimeUtil tm1, tm2 ;
                time_t start, end;
                TimeUtil start_tm(item_info[1]);
                //Dbc *cur_array[3], *join_curs;
                Dbt join_key, join_data;

                tm1 = start_tm;
                start = tm1.GetTimeT();

                if (item_info.size() > 2) {
                    TimeUtil end_tm = TimeUtil(item_info[2]);
                    tm2 = end_tm;
                    end = tm2.GetTimeT();
                } else {
                    end = 0;
                }               
                if (end) {
                    if (end < start) {
                        time_t temp = end;
                        end = start;
                        start = temp;
                        end = tm1.GetNextNthDayT(1);
                    } else {
                        end = tm2.GetNextNthDayT(1);
                    }
                } 

                fromDateKey.set_data(&start);
                fromDateKey.set_size(sizeof(start));

                if (cursor_date->get(&fromDateKey, &primData, DB_SET_RANGE) != 0)
                    throw "record not found";
                do {
                    Record *r = (Record *)primData.get_data();
                    if (end && r->date >= end)
                        break;
                    if (!strcmp(r->item, item_info[0].c_str())) {
                        record_list.push_back(*r);
                        total_cost += r->price;
                    }
                } while (cursor_date->get(&fromDateKey, &primData, DB_NEXT) == 0);
                
            } catch (const char *str) {
                throw str;
            }

        }
    } catch (DbException& e) {          
        cout << "Problem " << e.what() << endl;
        retval = -1;
    } catch (...) {
        cout << "Some problem in getting item info \n";
        retval = -1;
    }

    if ((retval != -1) &&  (record_list.size() > 0)) {
       record_list.sort(reverse_recordComparator);
       if (true == show_and_clear) {
           list<Record>::iterator it = record_list.begin();

           while (it != record_list.end()){
               show_put_record(*it);
               it++;
           }
           record_list.clear();
       }
    }
    
    if (cursor_name!= NULL)
        cursor_name->close();
    if (cursor_date != NULL)
        cursor_date->close();
    
    return retval;
   
}

//
//  On success retunrns number of entries deleted
//  on failure returns -1
//

int database::delete_entries(string& item, 
        string& from_date, string& to_date)
{
    Dbc *name_cursor = NULL, *date_cursor = NULL;
    double change_in_total_cost = 0;
    int ret, item_deleted = 0;
    Record *r = NULL;
    
    if (item.size() == 0 || from_date.size() == 0 || to_date.size() == 0)
        return -1;

    try {
        Dbt dKey((void *)item.c_str(), item.size() + 1);    
        Dbt dData;
        
        TimeUtil tm1(from_date);
        TimeUtil tm2(to_date);

        time_t t1 = tm1.GetTimeT(),
               t2 = tm2.GetTimeT(), temp_time;
        
        if (t1 > t2) {
           t1 = t2;
           t2 = tm1.GetNextNthDayT(1); 
        } else 
            t2 = tm2.GetNextNthDayT(1);

        new_db_sec->cursor(NULL, &name_cursor, 0);

        if (name_cursor->get(&dKey, &dData, DB_SET) != 0) {
            throw "Item  not found ";
        }

        //Position the date_cursor in proper place
        
        dKey.set_data(&t1);
        dKey.set_size(sizeof(t1));

        new_db_thi->cursor(NULL, &date_cursor, 0);
        
        date_cursor->get(&dKey, &dData, DB_SET_RANGE);
        
        temp_time = *((time_t *)dKey.get_data());

        if (temp_time > t2) {
            throw "No item found within date range";
        }
         
        dKey.set_data((void *)item.c_str());
        dKey.set_size(item.size() + 1);

        ret = name_cursor->get(&dKey, &dData, DB_SET);

        while (ret == 0){
            r = (Record *)dData.get_data();

            if (r->date >= t1 && r->date < t2) {
                change_in_total_cost += r->price;
                name_cursor->del(0);
                item_deleted++;
            } 
            ret = name_cursor->get(&dKey, &dData, DB_NEXT_DUP);
        } 
        ret = item_deleted;

    } catch (DbException& e) {

        cout << "Some problem in deleting entries " << e.what() << endl;
        ret = -1;
    } catch (const char *str) {
        cout << "Problem " << str << endl;
        ret = -1;
    } catch (...) {
        cout << "Some problem in deleting entries " << endl;
        ret = -1;
    }
    
    if (name_cursor)
        name_cursor->close();

    if (date_cursor)
        date_cursor->close();
    
    if (change_in_total_cost != 0)
        add_to_total_cost(- change_in_total_cost);

    return ret;
}


int database::put_record(list<Record> &rs)
{
    list<Record>::iterator it = rs.begin();
    Record dt; 
    Dbt key;
    Dbt data;
    unsigned int choice;
    double total_price = 0;

    
    system("clear");
    while (it != rs.end()){
        show_put_record(*it);
        total_price += it->price;
        it++;
    }
    cout << "\n\n    Total price " << total_price  << endl << endl;
    cout << " ARE THE ABOVE ENTRIES CORRECT,  ENTER 1 for yes,  0 NO ";
    cin >> choice ;
    if (choice == 0)
           return -1; 

    it = rs.begin();
    try {
        int rec_no;
        int key_no = REC_NO_KEY;
        double previous_cost;
        Dbt dKey, dData;

        dKey.set_data(&key_no);
        dKey.set_size(sizeof(key_no));

        rec_no_db->get(NULL, &dKey, &dData, 0);
        rec_no = *((int *)dData.get_data());

        key_no = 1;

        total_cost_db->get(NULL, &dKey, &dData, 0);
        previous_cost = *((double *)dData.get_data());
        
        while (it != rs.end()) {
            show_put_record(*it);
            memcpy(dt.item, it->item, ITEM_SIZE);
            dt.price = it->price;
            dt.date = it->date;
            key.set_data(&rec_no);
            key.set_size(sizeof(rec_no));
            data.set_data(&dt);
            data.set_size(sizeof(Record));
            new_db->put(NULL, &key, &data, 0);
            rec_no++;
            it++;
        } 
        
        previous_cost += total_price;

        dData.set_size(sizeof(previous_cost));
        dData.set_data(&previous_cost);
        key_no = 1;
        dKey.set_size(sizeof(key_no));
        dKey.set_data(&key_no);

        total_cost_db->put(NULL, &dKey, &dData, 0);

        dData.set_size(sizeof(rec_no));
        dData.set_data(&rec_no);
        key_no = REC_NO_KEY;

        rec_no_db->put(NULL, &dKey, &dData, 0);

    } catch(DbException &e) {
        cerr << "Error in puttingh data " << e.what() << endl;
        return -1;
    } catch (...) {
        cerr << "Error in puttingh data " << endl;
        return -1;
    }
    return 0;
}

void database::add_records(string& date)
{
   string item;
   float price;
   time_t current;
   int i,j;
   struct tm tmval;
   list<Record> lst;
   Record add_record;


   memset(&tmval, 0 , sizeof(struct tm));
   if (!strptime(date.c_str(), "%d-%b-%y", &tmval)) {
       cerr << "  Incorrect date str \n";
       return ;
   }
   current = mktime(&tmval);
   cout << "   Enter items and price  for " << date << endl;
   cout << "   To fiinish entering, enter 0 for item 0 for price\n\n";

   try {
       while (1) {
            cout << "Item  Price  ";
            price = 0;
            item = "";
            date = "0";
            cin >>  item  >> price;       
            if (item == "0" && price == 0 )
                    break;
            strncpy(add_record.item, item.c_str(), 19);
            i = strlen(add_record.item );
            for ( j = 0 ; j < i ; j++)
                add_record.item[j] = toupper(add_record.item[j] );
            add_record.price = price;
            add_record.date = current;
            lst.push_back(add_record);
       }
       if (lst.size() == 0)
               return;
       this->put_record(lst);
   } catch(...) {
      cerr << "There was some error in adding the data \n";
   }
}

void database::show_database_records()
{
    list<Record> lst;
    float total_price = 0 , total_cost;
    int rec_no = 1;
    Dbt dKey, dData;

    try {

        Dbt total_cost_key, rec_no_key;
        this->get_record(lst);

        // Now get the record number
        Dbc *rec_cursor_p;
        rec_no_db->cursor(NULL, &rec_cursor_p, 0);
        rec_cursor_p->get(&rec_no_key, &dData, DB_NEXT);
        rec_no = *((int *)dData.get_data());
        cout << "Number of record added " << rec_no << endl;

        Dbc *cost_cursor_p;
        total_cost_db->cursor(NULL, &cost_cursor_p, 0);
        cost_cursor_p->get(&total_cost_key, &dData, DB_NEXT);
        total_cost = *((float *)dData.get_data());
        cout << "Total cost " << total_cost << endl;


        if (lst.size() < 0)
            return;
        lst.sort(recordComparator);

        list<Record>::iterator it = lst.begin();
        while (it != lst.end()) {
            show_put_record(*it);
            it++;
        }
        rec_no_db->close(0);
        cost_cursor_p->close();    

    } catch (DbException& e) {
        cout << e.what() << endl;
        return ;
    } catch (...) {
        cout << "Some problem \n";        
    }

    cout << "\n\n Total Price " << total_price << "\n\n";
}

double database::add_to_total_cost(double extra)
{
    Dbt data;
    int rec_no = 1;
    Dbt key(&rec_no, sizeof(rec_no));
    
    total_cost_db->get(NULL, &key, &data, 0);
    double total_cost = *((double *)data.get_data());
    
    total_cost += extra;
    data.set_data(&total_cost);
    data.set_size(sizeof(total_cost));
    
    total_cost_db->put(NULL, &key, &data, 0); 
   
    return total_cost;
}

double database::get_total_cost()
{
    Dbt data;
    int rec_no = 1;
    Dbt key(&rec_no, sizeof(rec_no));
    
    total_cost_db->get(NULL, &key, &data, 0);
    double total_cost = *((double *)data.get_data());
    
    cout << total_cost << endl;

    return total_cost;
}

int database::add_delete_category(string& category_name,
        bool add)
{
    int ret_val = -1, cat_id;
    string cat_str = category_name;
    Dbt dKey, dData;
    Dbc *category_cursor = NULL;

    if (cat_str.size()  == 0 && true == add)
        return -1;

    dKey.set_size(cat_str.size() + 1);
    dKey.set_data((void *)cat_str.c_str());

    try {
       
        if (cat_str.size() > 0) 
            ret_val = category_db->get(NULL,&dKey, &dData, 0); 
        
        if (ret_val == 0)
            cat_id = *((int *) dData.get_data());

        if (true == add &&  ret_val == 0) {
            cout << "Category " << category_name << 
                " already added\n";
            return -1;

        } else if (false == add && cat_str.size() > 0 && ret_val != 0) {
            cout << "Category " << cat_str << 
                " not found \n";
            return -1;
        } else {
            int cat_no_key = CATEGORY_NO_KEY;
            int cat_no_val;
            Dbt catKey(&cat_no_key, sizeof(cat_no_key));

            rec_no_db->get(NULL, &catKey, &dData, 0);
            cat_no_val = *((int *) dData.get_data());

            if (true == add){
                int data = 0;
                cat_no_key = CATEGORY_NEXT_ID; 

                rec_no_db->get(NULL, &catKey, &dData, 0);
                
                data = *((int *)dData.get_data());
                
                cout << "Next Record Id was " << data << endl;

                dData.set_size(sizeof(data));
                dData.set_data(&data);
                category_db->put(NULL, &dKey, &dData, 0);
                cat_no_val++;
                data++;
                
                rec_no_db->put(NULL,&catKey, &dData, 0);
                
            } else  {
                if (cat_str.size() > 0) {                   
                    category_db->del(NULL, &dKey, 0);

                    dKey.set_data(&cat_id);
                    dKey.set_size(sizeof(cat_id));
                    
                    item_category_db->cursor(NULL, &category_cursor, 0);

                    while (category_cursor->get(&dKey, &dData, DB_SET) == 0) 
                        category_cursor->del(0);

                    cat_no_val--;

                } else {

                    // delete all categories 
                    Dbt tempKey(&cat_id, sizeof(cat_id));
                    Dbc *item_cat_cursor;

                    item_category_db->cursor(NULL, &item_cat_cursor, 0);

                    memset(&dKey, 0 , sizeof(dKey));
                    category_db->cursor(NULL, &category_cursor, 0);
                    
                    while (category_cursor->get(&dKey,&dData, DB_NEXT) == 0) {
                        cat_id = *((int *)dData.get_data());
                        
                        while (item_cat_cursor->get(&tempKey, &dData, DB_SET)
                                == 0) 
                           item_cat_cursor->del(0); 

                        category_cursor->del(0);
                    }                    

                    item_cat_cursor->close();
                    cat_no_val = 0;

                }
            } 
            
            cat_no_key = CATEGORY_NO_KEY;
            dData.set_data(&cat_no_val);
            dData.set_size(sizeof(cat_no_val));
            
            rec_no_db->put(NULL, &catKey, &dData, 0);
            
            if (0 == cat_no_val) {
                cat_id = 1;
                cat_no_key = CATEGORY_NEXT_ID;
                dData.set_data(&cat_id);
                dData.set_size(sizeof(cat_id));
                rec_no_db->put(NULL, &catKey, &dData, 0);
            }

            ret_val = 0;
        }
      
    } catch (DbException& e) {
        cout << " Exception in adding category " << e.what() 
            << endl;
    } catch (...) {
        cout << "Exception in adding category " << endl;
    }
    
    if (category_cursor)
        category_cursor->close();

    return ret_val;
}

// 
// Returns the number of category displayed 
// If category not found a failure, i.e. -1 
// is returned 
//

int database::show_category(string category_name)
{
    Dbt dKey, dData;
    Dbc *category_cursor = NULL;
    int data_found = 0;

    try {

        if (category_name == "") {
            // show the category list 

            category_db->cursor(NULL, &category_cursor , 0 );
            while (category_cursor->get(&dKey, &dData, DB_NEXT) == 0) {
                char *cat_name = (char *)dKey.get_data();
                cout << cat_name << endl;
                data_found++; 
            } 
        } else {
            string my_cat = category_name;

            dKey.set_data((void *)my_cat.c_str());
            dKey.set_size(my_cat.size() + 1);

            if (category_db->get(NULL, &dKey, &dData, 0) == 0){
                char *cat_name = (char *)dKey.get_data();
                int ret_val;

                data_found = 1;    

                cout << "Category   "  <<  cat_name << endl;

                item_category_db->cursor(NULL, &category_cursor, 0);

                ret_val = category_cursor->get(&dData, &dKey, DB_SET);
                
                while (ret_val == 0) {
                    cat_name = (char *)dKey.get_data();
                    cout << "     " << cat_name << endl;
                    ret_val = category_cursor->get(&dData, &dKey, DB_NEXT_DUP);
                } 
            } else {
                cout << "Category " << category_name << " not found\n";
            }
        }
    } catch (DbException& e) {
        cout << "Problem: " << e.what() << endl; 
        data_found = -1;
    } catch (...) {
        cout << "Some problem\n";
        data_found = -1;
    }
    
    if (category_cursor)
        category_cursor->close();

    return data_found;
}

//
// database::insert_delete_from_category
// returns 0 on success, -1 on failure
//
int database::insert_delete_from_category(
        string& category_name,
        string& item_name,
        bool add)
{
    int ret_val = -1, category_id;
    string cat_str = category_name;
    string item_str =  item_name;
    Dbt dKey, dData;
    
    // first see, if the category exists...

    try {
        dKey.set_size(cat_str.size() + 1);
        dKey.set_data((void *)cat_str.c_str());

        ret_val = category_db->get(NULL,&dKey, &dData, 0);
        
        if (ret_val != 0) { 
            cout << "Category " << cat_str << " not found\n";
            return -1;
        }
        category_id = *((int *) dData.get_data());

        // now see if the item exists 
        
        dKey.set_size(item_str.size() + 1);
        dKey.set_data((void *)item_str.c_str());

        ret_val = new_db_sec->get(NULL, &dKey, &dData, 0);   

        if (ret_val != 0){
            cout << "Item " << item_str << " not found \n";
            return -1;
        }
        
        // Now add/delete from the item_category_db this pair

        dKey.set_data(&category_id);
        dKey.set_size(sizeof(category_id));

        Dbc *cat_item_cursor;
        
        item_category_db->cursor(NULL, &cat_item_cursor, 0);
        
        ret_val = cat_item_cursor->get(&dKey, &dData, DB_SET);
        
         while (0 == ret_val) {
             char *temp_item = (char *)dData.get_data();     
             if (item_str == temp_item) {
                if (false == add) {
                    cat_item_cursor->del(0);
                    cat_item_cursor->close();
                    return 0;
                } else {
                    cat_item_cursor->close();
                    cout << "Category " << cat_str << " already has item "
                            << item_str << endl;
                    return -1;
                }  
             }
             ret_val = cat_item_cursor->get(&dKey, &dData, DB_NEXT_DUP);
         } 

         if (true == add) {
            dData.set_data((void *)item_str.c_str());
            dData.set_size(item_str.size() + 1);

            item_category_db->put(NULL, &dKey, &dData, 0);

            return 0;   
         }
    } catch (DbException& e) {
        cout << __FUNCTION__ << __LINE__ << 
           " " << e.what() << endl; 
        return -1;
    } catch (...) {
        cout << __FUNCTION__ << __LINE__ <<
             "  some problem \n";
        return -1;
    }
    return 0;
}


