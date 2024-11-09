#include <iostream>
#include <thread>
#include <cstdlib>
#include "../db_server.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <map>

#endif

void connectionHandler(int new_socket, Server& db_server, std::map<std::string, Database*>& databases);
void exitHandler();
void tcp_init(const uint16_t PORT, Server& db_server, std::map<std::string, Database*>& databases);