#ifndef DATABASE
#define DATABASE
#include <map>
#include <string>
#include <vector>
#include <optional>

using namespace std;

struct Table {
    // The map object itself represents each column in the table.
    // The key represents the name of the column (for example; "Name", "Date", etc)
    // The vector represents all rows within the column
    map<string, vector<string>> data; 
};

class Database {
    public:
        // The databases map keeps track of all existing databases to prevent duplicates
        static map<string, Database*> databases;
        map<string, Table> tables;
        string name;
        Database(string &databaseName);
        int addTable(string &tableName, string columnNames[], size_t &size);
        int addRow(string &tableName, string values[], size_t &size);
        int deleteRow(string &tableName, unsigned int &rowNumber);
        string getCell(string &tableName, unsigned int &rowNumber, string &columnName);
        int overwriteRow(string &tableName, unsigned int &rowNumber, string values[], size_t &size);
        map<string, string> getRow(string &tableName, unsigned int &rowNumber);
        int overwriteCell(string &tableName, unsigned int &rowNumber, string &columnName, string &newValue);
        int addColumn(string &tableName, string &columnName);
        int deleteColumn(string &tableName, string &columnName);
        int deleteTable(string &tableName);
        int renameTable(string &oldName, string &newName);
        int renameColumn(string &tableName, string &oldName, string &newName);
        map<string, string> save();
};

#endif