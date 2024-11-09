#include <iostream>
#include <vector>
#include "database.h"
#include "statuscodes.h"
#include "tcp/server.h"
#include "db_server.h"
#include <filesystem>

namespace fs = std::filesystem;

int main() {
   auto server = Server();
   std::thread tcp(tcp_init, 6969, std::ref(server), std::ref(Database::databases));
   tcp.detach();

   while (true) {};

   return 0;
}