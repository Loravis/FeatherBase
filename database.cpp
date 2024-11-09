#include <iostream>
#include <vector>
#include <map>
#include "statuscodes.h"
#include "database.h"

using namespace std;

map<string, Database*> Database::databases;

/**The constructor uses the databases map to check for a duplicate.
 * EXISTS is thrown if a database with the same key exists. */
Database::Database(string &databaseName) {
    if (Database::databases.find(databaseName) != Database::databases.end()) {
        throw(EXISTS);
    } else {
        name = databaseName;
        Database::databases[name] = this;
        tables = {};
    }
};

//Note: Each column must have a unique name.
int Database::addTable(string &tableName, string columnNames[], size_t &size) {
    if (tables.find(tableName) != tables.end()) {
        return EXISTS;
    } else {
        Table table;
        for (size_t i = 0; i < size; i++) {
            vector<string> emptyDataVector;
            table.data[columnNames[i]] = emptyDataVector;
        }
        tables[tableName] = table;
        return SUCCESS;
    } 
};

/**Adds a new row to a table and returns the row number.
 * Throws NOTFOUND if the table doesn't exist. 
 * Throws INVALID if the size of the values array doesn't match the table's row size*/
int Database::addRow(string &tableName, string values[], size_t &size) {
    if (tables.find(tableName) == tables.end()) {
        return NOTFOUND;
    } else {
        Table &table = tables[tableName];
        if (size > 0 and size == table.data.size()) {
            int i = 0;
            for (auto [key, val] : table.data) {
                auto &col = table.data[key];
                col.insert(col.end(), values[i]);
                i++;
            }
            return table.data.begin()->second.size() - 1;
        } else {
            return INVALID;
        }
    }
};

/**Returns the data from the specified table row.
 * Throws NOTFOUND if the table doesn't exist. 
 * Throws INVALID if the specified row doesn't exist.
 * NOTE: Row numbers start at 0.*/
map<string, string> Database::getRow(string &tableName, unsigned int &rowNumber) {
    if (tables.find(tableName) == tables.end()) {
        throw(NOTFOUND);
    } else {
        Table &table = tables[tableName];
        if (table.data.begin()->second.size() > rowNumber) {
            map<string, string> returnRow;
            for (auto const& [key, val] : table.data) {
                returnRow[key] = val[rowNumber];
            }
            return returnRow;
        } else {
            throw(INVALID);
        }
    }
};

/**Deletes a specified row and returns SUCCESS.
 * Throws NOTFOUND if the table doesn't exist. 
 * Throws INVALID if the specified row doesn't exist.
 * NOTE: Row numbers start at 0.*/
int Database::deleteRow(string &tableName, unsigned int &rowNumber) {
    if (tables.find(tableName) == tables.end()) {
        throw(NOTFOUND);
    } else {
        Table &table = tables[tableName];
        if (table.data.begin()->second.size() > rowNumber) {
            for (auto const& [key, val] : table.data) {
                table.data[key].erase(table.data[key].begin() + rowNumber);
            }
            return SUCCESS;
        } else {
            return INVALID;
        }
    }
};

/**Overwrites a specified row.
 * Returns NOTFOUND if the table doesn't exist. 
 * Returns INVALID if the size of the values array doesn't match the table's row size
 * or if the specified row doesn't exist.*/
int Database::overwriteRow(string &tableName, unsigned int &rowNumber, string values[], size_t &size) {
    if (tables.find(tableName) == tables.end()) {
        return NOTFOUND;
    } else {
        Table &table = tables[tableName];
        std::cout << size << std::endl;
        if (size > 0 and size == table.data.size() and table.data.begin()->second.size() > rowNumber) {
            int i = 0;
            for (auto [key, val] : table.data) {
                auto &col = table.data[key];
                col.insert(col.begin() + rowNumber, values[i]);
                i++;
            }
        } else {
            return INVALID;
        }
        return SUCCESS;
    }
};

/**Returns data from a specific cell in the table.
 * Throws INVALID if either row or column do not exist.
 * Throws NOTFOUND if the table doesn't exist.
 */
string Database::getCell(string &tableName, unsigned int &rowNumber, string &columnName) {
    if (tables.find(tableName) == tables.end()) {
        throw(NOTFOUND);
    } else {
        Table &table = tables[tableName];
        if (table.data.begin()->second.size() > rowNumber and table.data.find(columnName) != table.data.end()) {
            return table.data[columnName][rowNumber];
        } else {
            throw(INVALID);
        }
    }
};

/**Overwrites a specific cell in the table.
 * Throws INVALID if either row or column do not exist.
 * Throws NOTFOUND if the table doesn't exist.
 */
int Database::overwriteCell(string &tableName, unsigned int &rowNumber, string &columnName, string &newValue) {
    if (tables.find(tableName) == tables.end()) {
        return NOTFOUND;
    } else {
        Table &table = tables[tableName];
        if (table.data.begin()->second.size() > rowNumber and table.data.find(columnName) != table.data.end()) {
            table.data[columnName][rowNumber] = newValue;
        } else {
            return INVALID;
        }
        return SUCCESS;
    }
};

//Adds a new column to an existing table.
int Database::addColumn(string &tableName, string &columnName) {
    if (tables.find(tableName) == tables.end()) {
        return NOTFOUND;
    } else {
        Table &table = tables[tableName];

        if (table.data.find(columnName) != table.data.end()) {
            return EXISTS;
        }

        int columnSize = 0;
        //This if statement ensures that the new column contains the 
        //same number of rows of all other columns, if other columns exist.
        if (table.data.size() > 0) {
            columnSize = table.data.begin()->second.size();
        }
        vector<string> emptyDataVector;
        auto &column = table.data[columnName];
        column = emptyDataVector;
        for (int i = 0; i < columnSize; i++) {
            column.insert(column.end(), "");
        } 

        return SUCCESS;
    }
};


/**Deletes a specified column.*/
int Database::deleteColumn(string &tableName, string &columnName) {
    if (tables.find(tableName) == tables.end()) {
        return NOTFOUND;
    } else {
        Table &table = tables[tableName];
        if (table.data.find(columnName) == table.data.end()) {
            return INVALID;
        } else {
            table.data.erase(columnName);
            return SUCCESS;
        }
    }
}

//Delete a table.
int Database::deleteTable(string &tableName) {
    std::cout << tableName << "\n";
    if (tables.find(tableName) == tables.end()) {
        return NOTFOUND;
    } else {
        tables.erase(tableName);
        return SUCCESS;
    }
}

int Database::renameTable(string &oldName, string &newName) {
    if (tables.find(oldName) == tables.end()) {
        return NOTFOUND;
    } else {
        if (tables.find(newName) != tables.end()) {
            return EXISTS;
        }

        tables[newName] = tables[oldName];
        tables.erase(oldName);
        return SUCCESS;
    }
}

int Database::renameColumn(string &tableName, string &oldName, string &newName) {
    if (tables.find(tableName) == tables.end()) {
        return NOTFOUND;
    } else {
        Table &table = tables[tableName];
        if (table.data.find(newName) != table.data.end()) {
            return EXISTS;
        }

        if (table.data.find(oldName) == table.data.end()) {
            return INVALID;
        }

        table.data[newName] = table.data[oldName];
        table.data.erase(oldName);

        return SUCCESS;
    }
}

map<string, string> Database::save() {
    map<string, string> result = {};
    for (auto const& [tablename, table] : tables) {
        string tablestring = "";
        for (auto const& [colname, coldata] : table.data) {
            tablestring += char(1);
            tablestring.append(colname);
            tablestring += char(2);
            for (string data : coldata) {
                tablestring.append(data);
                tablestring += char(9);
            }
            if (tablestring.back() == char(9)) {
                tablestring.pop_back();
            }
            tablestring += char(3);
        }
        result[tablename] = tablestring;
    }

    return result;
}