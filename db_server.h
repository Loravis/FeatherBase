#ifndef DATABASE_SERVER_MODULE
#define DATABASE_SERVER_MODULE
#include <iostream>
#include "database.h"
#include <map>

class Server {
    public:
        Server();
        std::string interpretCommand(string &command, std::map<std::string, Database*>& databases);
};

#endif