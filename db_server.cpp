#include "db_server.h"
#include "database.h"
#include "statuscodes.h"
#include <map>
#include <list>
#include <mutex>
#include <cstdlib>

std::mutex mtx;

Server::Server() {
    // TODO: File reading and writing

    std::cout << "Server ready\n";
}

//Command interpreter
std::string Server::interpretCommand(string &command, std::map<std::string, Database*>& databases) {
    //Characters before the first SOH character are the name of the operation
    int firstSOH = command.find(1);

    if (firstSOH == std::string::npos) {
        return "SYNTAXERROR";
    }

    std::string operation = command.substr(0, firstSOH);
    command.erase(0, firstSOH + 1);

    //Add a new table to a database
    //addtbl\x01dbname\x01tablename\x02colname\x09colname\x09colname\x04
    if (operation == "addtbl") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);
        
        std::vector<std::string> columnNames;

        while (data.length() > 1) {
            if (data.find(9) == std::string::npos) {
                columnNames.insert(columnNames.end(), data.substr(0, data.find(4)));
                data.erase(0, data.find(4) + 1);
            } else {
                columnNames.insert(columnNames.end(), data.substr(0, data.find(9)));
                data.erase(0, data.find(9) + 1);
            }
        }

        std::string* columnNamesArray = &columnNames[0];
        size_t columns = columnNames.size();
        mtx.lock();
        int statuscode = databases[databasename]->addTable(tablename, columnNamesArray, columns);
        mtx.unlock();

        if (statuscode != 0) {
            return getStatusCodeString(statuscode);
        }
    }
    //Create a new database
    //createdb\x01dbname\x04
    else if (operation == "createdb") {
        int endOfTransmission = command.find(4);

        if (endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }
        std::string databasename = command.substr(0, endOfTransmission);
        
        try {
            mtx.lock();
            databases[databasename] = new Database(databasename);
            mtx.unlock();
        } catch (exception ex) {
            return "EXISTS";
        }
    }
    //Get existing databases
    //getdbs\x01\x04
    //Returns dbname\x09dbname\x09dbname\x04
    else if (operation == "getdbs") {
        if (databases.empty()) {
            return "NOTFOUND";
        } else {
            std::string databasenames;
            bool databasesExist = false;
            for(std::map<std::string, Database*>::iterator it = databases.begin(); it != databases.end(); ++it) {
                //Get database name and add it to the response string
                databasenames.append(it->first);
                databasenames += char(9);
                databasesExist = true;
            }
            if (databasesExist == false) {
                return "";
            }
            //Ensure the final character in the string is \x04 (end of transmission)
            databasenames.erase(databasenames.size()-1);
            databasenames += char(4);

            return databasenames;
        }
    }
    //Get existing tables
    //gettbls\x01dbname\x04
    //Returns tablename\x09tablename\x09tablename\x04
    else if (operation == "gettbls") {
        int endOfTransmission = command.find(4);
        if (endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }
        std::string databasename = command.substr(0, endOfTransmission);

        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        } 

        std::map<std::string, Table> tables = databases[databasename]->tables;

        std::string tablenames;
        bool tablesExist = false;
        for(std::map<std::string, Table>::iterator it = tables.begin(); it != tables.end(); ++it) {
            //Get table name and add it to the response string
            tablenames.append(it->first);
            tablenames += char(9);
            tablesExist = true;
        }
        if (tablesExist == false) {
            return "";
        }

        //Ensure the final character in the string is \x04 (end of transmission)
        tablenames.erase(tablenames.size()-1);
        tablenames += char(4);

        return tablenames;
    }
    //deltbl\x01dbname\x02tablename\x04
    else if (operation == "deltbl") {
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dataStart); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string data = command.substr(dataStart + 1, endOfTransmission - dataStart - 1);

        mtx.lock();
        int statuscode = databases[databasename]->deleteTable(data);
        mtx.unlock();
        
        std::cout << "test\n";

        if (statuscode != 0) {
            return getStatusCodeString(statuscode);
        }
    }
    //deldb\x01dbname\x04
    else if (operation == "deldb") {
        int endOfTransmission = command.find(4);

        if (endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, endOfTransmission); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        mtx.lock();
        databases.erase(databasename);
        mtx.unlock();
        return "SUCCESS";
    }
    //renamedb\x01dbname\x02newname\x04
    else if (operation == "renamedb") {
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dataStart); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string data = command.substr(dataStart + 1, endOfTransmission - dataStart - 1);

        mtx.lock();
        if (databases.find(data) != databases.end()) {
            return "EXISTS";
        }

        databases[data] = databases[databasename];
        databases.erase(databasename);
        mtx.unlock();
        
        return "SUCCESS";
    }
    //addrow\x01dbname\x01tablename\x02data\x09data\x09data\x04
    else if (operation == "addrow") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);
        
        std::vector<std::string> columnData;

        while (data.length() > 1) {
            if (data.find(9) == std::string::npos) {
                columnData.insert(columnData.end(), data.substr(0, data.find(4)));
                data.erase(0, data.find(4) + 1);
            } else {
                columnData.insert(columnData.end(), data.substr(0, data.find(9)));
                data.erase(0, data.find(9) + 1);
            }
        }

        std::string* columnDataArray = &columnData[0];
        size_t columns = columnData.size();
        mtx.lock();
        int statuscode = databases[databasename]->addRow(tablename, columnDataArray, columns);

        if (statuscode != 0) {
            return getStatusCodeString(statuscode);
        }
        mtx.unlock();
    }
    //getcell\x01dbname\x01tablename\x02ROWNUMBER\x09columnname\x04
    else if (operation == "getcell") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);
        
        unsigned int rownumber;
        int tabulator = data.find(9);
        if (tabulator == std::string::npos) {
            return "SYNTAXERROR";
        }

        //This try-catch block ensures that a non-numerical row number parameter results in SYNTAXERROR being returned.
        try {
            rownumber = std::stoi(data.substr(0, tabulator));
        } catch (exception ex) {
            return "SYNTAXERROR";
        }
        
        //Retrieve the column name. 
        std::string columnname = data.substr(tabulator + 1, data.length() - 3 - tabulator);
        try {
            std::string result = databases[databasename]->getCell(tablename, rownumber, columnname);
            return result;
        } catch (int code) {
            return getStatusCodeString(code);
        }
    }
    //getrow\x01dbname\x01tablename\x02ROWNUMBER\x04
    //Returns value\x09value\x09value\x04
    else if (operation == "getrow") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);
        
        unsigned int rownumber;
        //This try-catch block ensures that a non-numerical row number parameter results in SYNTAXERROR being returned.
        try {
            rownumber = std::stoi(data.substr(0, data.find(4)));
        } catch (exception ex) {
            return "SYNTAXERROR";
        }

        std::map<std::string, std::string> row = databases[databasename]->getRow(tablename, rownumber);
        std::string values;
        for (auto const& [columnname, val] : row) {
            values.append(val);
            values += char(9);
        }
        values.erase(values.size()-1);
        values += char(4);
        return values;
    }
    //owrow\x01dbname\x01tablename\x05ROWNUMBER\x02data\x09data\x09data\x04
    else if (operation == "owrow") {
        int dbnamePos = command.find(1);
        int enquiryPos = command.find(5);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos
        or enquiryPos == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, enquiryPos - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);

        unsigned int rownumber;
        //This try-catch block ensures that a non-numerical row number parameter results in SYNTAXERROR being returned.
        try {
            rownumber = std::stoi(command.substr(enquiryPos + 1, dataStart));
        } catch (exception ex) {
            return "SYNTAXERROR";
        }

        std::cout << rownumber << std::endl;
        
        std::vector<std::string> columnData;

        while (data.length() > 1) {
            if (data.find(9) == std::string::npos) {
                columnData.insert(columnData.end(), data.substr(0, data.find(4)));
                data.erase(0, data.find(4) + 1);
            } else {
                columnData.insert(columnData.end(), data.substr(0, data.find(9)));
                data.erase(0, data.find(9) + 1);
            }
        }

        std::string* columnDataArray = &columnData[0];
        size_t columns = columnData.size();
        mtx.lock();
        int statuscode = databases[databasename]->overwriteRow(tablename, rownumber, columnDataArray, columns);
        mtx.unlock();

        if (statuscode != 0) {
            return getStatusCodeString(statuscode);
        }
    }
    //addcol\x01dbname\x01tablename\x02colname\x04
    else if (operation == "addcol") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);

        mtx.lock();
        int statuscode = databases[databasename]->addColumn(tablename, data);
        mtx.unlock();

        if (statuscode != SUCCESS) {
            return getStatusCodeString(statuscode);
        }
    }
    //delcol\x01dbname\x01tablename\x02colname\x04
    else if (operation == "delcol") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);

        mtx.lock();
        int statuscode = databases[databasename]->deleteColumn(tablename, data);
        mtx.unlock();

        if (statuscode != SUCCESS) {
            return getStatusCodeString(statuscode);
        }
    }
    //renametbl\x01dbname\x01tablename\x02newname\x04
    else if (operation == "renametbl") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);

        mtx.lock();
        int statuscode = databases[databasename]->renameTable(tablename, data);
        mtx.unlock();

        if (statuscode != SUCCESS) {
            return getStatusCodeString(statuscode);
        }
    }
    //renamecol\x01dbname\x01tablename\x02colname\x09newname\x04
    else if (operation == "renamecol") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);
        
        unsigned int rownumber;
        int tabulator = data.find(9);
        if (tabulator == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string columnname = data.substr(0, tabulator);
        
        //Retrieve the new column name. 
        std::string newname = data.substr(tabulator + 1, data.length() - 3 - tabulator);
        std::cout << columnname << std::endl;
        std::cout << newname << std::endl; 
        
        mtx.lock();
        int statuscode = databases[databasename]->renameColumn(tablename, columnname, newname);
        mtx.unlock();
        if (statuscode != SUCCESS) {
            return getStatusCodeString(statuscode);
        }
    }
    //owcell\x01dbname\x01tablename\x02ROWNUMBER\x09columnname\x05newcontent\x04
    else if (operation == "owcell") {
        int dbnamePos = command.find(1);
        int dataStart = command.find(2);
        int enquiryPos = command.find(5);
        int endOfTransmission = command.find(4);

        if (dbnamePos == std::string::npos or dataStart == std::string::npos or endOfTransmission == std::string::npos
        or enquiryPos == std::string::npos) {
            return "SYNTAXERROR";
        }

        std::string databasename = command.substr(0, dbnamePos); //Get database name
        if (databases.find(databasename) == databases.end()) {
            return "NOTFOUND";
        }

        std::string tablename = command.substr(dbnamePos + 1, dataStart - (dbnamePos + 1)); //Get table name
        std::string data = command.substr(dataStart + 1, endOfTransmission - 1);
        enquiryPos = data.find(5);
        
        unsigned int rownumber;
        int tabulator = data.find(9);
        if (tabulator == std::string::npos) {
            return "SYNTAXERROR";
        }

        //This try-catch block ensures that a non-numerical row number parameter results in SYNTAXERROR being returned.
        try {
            rownumber = std::stoi(data.substr(0, tabulator));
        } catch (exception ex) {
            return "SYNTAXERROR";
        }
        
        //Retrieve the column name. 
        std::string columnname = data.substr(tabulator + 1, enquiryPos - 2);
        //Retrieve the new cell content.
        std::string newcontent = data.substr(enquiryPos + 1, data.length() - 4 - tabulator);
        newcontent.erase(newcontent.find(4));

        mtx.lock();
        int statuscode = databases[databasename]->overwriteCell(tablename, rownumber, columnname, newcontent);
        mtx.unlock();

        if (statuscode != SUCCESS) {
            return getStatusCodeString(statuscode);
        }
    }
    else {
        return "SYNTAXERROR";
    }
    return "SUCCESS";
}