# Featherbase

This is a no-SQL Database Management System. I designed it to be relatively simple and lightweight while offering all the base functionality you'd expect from a DBMS. I haven't yet gotten to finish this project, which is why some features are missing, however the features that were implemented are fully functional. 

✅ **Implemented features:**
- Create, manage and delete multiple databases
- Create, manage and delete tables
- Write, read and modify data
- Communicate with the database server via a TCP port

❌ **Not implemented features:**
- Save the database data to a file to prevent data loss on shutdown
- User management

## How to use
The database server will listen for TCP connections on port **6969**. You may use any TCP connection tool, like netcat for example, to open a connection and begin sending commands (strings consisting of specific unicode characters) to the DBMS. You may view the shell scripts in the **testing** sub-folder for examples on how to do this. A full command syntax list is found in the next section. 

## Commands
```c
createdb\x01dbname\x04 //Create a new database

getdbs\x01\x04 //Get a list of all existing databases. Each database name in the returned string is separated with \x09.

gettbls\x01dbname\x04 //Get a list of tables in a database. Each table name in the returned string is separated with \x09.

deltbl\x01dbname\x02tablename\x04 //Delete a table within a database

deldb\x01dbname\x04 //Delete an entire database

renamedb\x01dbname\x02newname\x04 //Rename a database

addrow\x01dbname\x01tablename\x02data\x09data\x09data\x04 //Adds a row of data to an existing table. If the amount of columns doesn't match, status code 3 (INVALID) is returned

getcell\x01dbname\x01tablename\x02ROWNUMBER\x09columnname\x04 //Returns the value of a specific cell in a table. ROWNUMBER must be an integer. 

getrow\x01dbname\x01tablename\x02ROWNUMBER\x04 //Get all values from a row in a table. Each value in the returned string is separated with \x09

owrow\x01dbname\x01tablename\x05ROWNUMBER\x02data\x09data\x09data\x04 //Overwrite data in a row in a table. If the amount of columns doesn't match, status code 3 (INVALID) is returned

addcol\x01dbname\x01tablename\x02colname\x04 //Add a column to a table

delcol\x01dbname\x01tablename\x02colname\x04 //Delete a column from a table

renametbl\x01dbname\x01tablename\x02newname\x04 //Rename a table

renamecol\x01dbname\x01tablename\x02colname\x09newname\x04 //Rename a column

owcell\x01dbname\x01tablename\x02ROWNUMBER\x09columnname\x05newdata\x04 //Overwrite a specific column. ROWNUMBER needs to be an integer.
```

## Status codes
Commands sent to the DBMS may return one of the following status codes;

- **SUCCESS (0)** : Returned whenever a command is successfully executed.
- **NOTFOUND (1)** : Returned whenever a requested resource, like a database or table, doesn't exist.
- **EXISTS (2)** : Returned when attempting to create a new resource, like a database or table, when it already exists.
- **INVALID (3)** : Returned when an operation is invalid. For example; writing a row with 6 columns to a table with only 3 columns would return this status code.
- **FAILURE (4)** : Currently unused.
- **SYNTAXERROR (5)** : Returned whenever a command has an incorrect syntax. See the "Commands" section for detailed information on each command and its syntax. 

## Compiling
You may use my custom build system to compile FeatherBase from source: https://github.com/Loravis/MiniMake

## Feedback & Contributions
If you wish to provide any feedback, suggest changes or contribute to this project, you may feel free to contact me via discord (@loravis) or email (address is linked to my GitHub profile). Any feedback would be greatly appreciated 🙂. 