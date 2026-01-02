/**
 * @file network.cpp
 * @brief Implementation of the Network class for TCP communication
 * @author Project CG - Maze Game
 * @date 2025
 */

#include "../include/Network.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

/**
 * @brief Creates a non-blocking TCP socket
 * @return Socket descriptor or -1 on error
 */
int Network::createSocket() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    cerr << "Error creating socket: " << strerror(errno) << endl;
  }
  return sock;
}

/**
 * @brief Binds the socket to a port and starts listening
 * @param socket Socket descriptor
 * @param port Port to listen on
 * @return true on success, false on error
 */
bool Network::bindAndListen(int socket, int port) {
  // Allow port reuse
  int opt = 1;
  setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = INADDR_ANY;

  if (::bind(socket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) <
      0) {
    cerr << "Bind error: " << strerror(errno) << endl;
    return false;
  }

  if (listen(socket, 1) < 0) {
    cerr << "Listen error: " << strerror(errno) << endl;
    return false;
  }

  return true;
}

/**
 * @brief Accepts an incoming connection
 * @param serverSocket Server socket descriptor
 * @return Client socket descriptor or -1 on error
 */
int Network::acceptConnection(int serverSocket) {
  int clientSocket = accept(serverSocket, nullptr, nullptr);
  if (clientSocket < 0) {
    cerr << "Error accepting connection: " << strerror(errno) << endl;
  }
  return clientSocket;
}

/**
 * @brief Connects to a remote server
 * @param socket Socket descriptor
 * @param ip Server IP address
 * @param port Server port
 * @return true on success, false on error
 */
bool Network::connectToServer(int socket, const string &ip, int port) {
  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);

  if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
    cerr << "Invalid address: " << ip << endl;
    return false;
  }

  if (connect(socket, (struct sockaddr *)&serverAddress,
              sizeof(serverAddress)) < 0) {
    cerr << "Error connecting to server: " << strerror(errno) << endl;
    return false;
  }

  return true;
}

/**
 * @brief Sends data over the socket
 * @param socket Socket descriptor
 * @param data Data buffer
 * @param size Size of data to send
 * @return true on success, false on error
 */
bool Network::sendData(int socket, const void *data, size_t size) {
  ssize_t bytesSent = send(socket, data, size, 0);
  if (bytesSent < 0) {
    cerr << "Error sending data: " << strerror(errno) << endl;
    return false;
  }
  return true;
}

/**
 * @brief Receives data from the socket
 * @param socket Socket descriptor
 * @param buffer Buffer to receive data
 * @param size Buffer size
 * @return Number of bytes received
 */
ssize_t Network::receiveData(int socket, void *buffer, size_t size) {
  return recv(socket, buffer, size, 0);
}

/**
 * @brief Closes the socket
 * @param socket Reference to socket descriptor (set to -1 after close)
 */
void Network::closeSocket(int &socket) {
  if (socket >= 0) {
    close(socket);
    socket = -1;
  }
}