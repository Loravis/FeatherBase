/**This file handles incoming TCP socket connections and passes commands to the DB Server. */

#include "server.h"
#include "../statuscodes.h"
#include "../database.h"

/**The connection handler accepts incoming TCP connections,
 * listens for any requests from the client and passes requests
 * to the command interpreter.
 */
void connectionHandler(int new_socket, Server& db_server, std::map<std::string, Database*>& databases) {
   char buffer[1024] = {0};
   const char* success = "Connection accepted\n";
   send(new_socket, success, strlen(success), 0);
   std::string command;

   /**This loop will repeatedly read bytes from the socket into a string.
    * If a newline character is read, the string is sent to the command interpreter 
    * and emptied after the command was executed.
    * Command interpretation is not done asynchronously.
    */
   while (true) {
      int valread = recv(new_socket, buffer, sizeof(buffer), 0);

      //Handle client disconnects
      if (valread == 0) {
         std::cout << "Client disconnected.\n";
         break;
      } else if (valread < 0) {
         std::cerr << "Receive error\n";
         break;
      }

      command.append(buffer, valread);
      
      //Interpret command
      if (buffer[valread - 1] == '\n') {
         std::string response;
         try {
            response = db_server.interpretCommand(command, databases);
         } catch (int errorcode) {
            /**Exception handling is necessary here, 
             * because database methods that return non-integer values
             * will throw a status code if an error occurs.
             */
            response = errorcode;
         }
         send(new_socket, response.c_str(), strlen(response.c_str()), 0); //Return a response to the client
         command = "";
      }
      memset(buffer, 0, sizeof(buffer));
   }

   close(new_socket);
}

int server_fd;

/**This function ensures that the TCP socket is correctly closed
 * when the server is shut down.
 */
void exitHandler() {
   #ifdef _WIN32
   closesocket(server_fd);
   WSACleanup();
   #else
   close(server_fd);
   #endif
}

/**Main TCP socket functionality.
 * This function sets up the TCP socket and passes any
 * connections to a new handler thread, to then be handled
 * by the connectionHandler function.
 */
void tcp_init(const uint16_t PORT, Server& db_server, std::map<std::string, Database*>& databases) {
   server_fd = socket(AF_INET, SOCK_STREAM, 0);
   if (server_fd == 0) {
      std::cerr << "Socket creation failed\n";
      exit(EXIT_FAILURE);
   }

   /**Ensure that the socket can be reused.
    * This prevents an issue where the server fails to bind the socket
    * to a port if FeatherBase is launched immediately after being shut down.
    */
   int opt = 1;
   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
   {
        std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
   }

   struct sockaddr_in address;
   int addrlen = sizeof(address);
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(PORT);
   if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
      std::cerr << "Bind failed\n";
      exit(EXIT_FAILURE);
   }

   if (listen(server_fd, 255) < 0) {
      std::cerr << "Listen failed\n";
      exit(EXIT_FAILURE);
   }
   
   bool listening = true;
   std::cout << "Server listening on port " << PORT << ".\n";

   //Ensure cleanup when server is shut down.
   std::atexit(exitHandler);

   //Constantly await any incoming TCP socket connections
   while (true) {
      int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
      if (new_socket < 0) {
         std::cerr << "Accept failed\n";
         exit(EXIT_FAILURE);
      }

      //Pass the connection to a new thread
      std::cout << "Client connected\n";
      std::thread connection(connectionHandler, new_socket, std::ref(db_server), std::ref(databases));
      connection.detach();
   }
}