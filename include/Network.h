/**
 * @file Network.h
 * @brief Utility class for TCP network communication
 * @author Project CG - Maze Game
 * @date 2025
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>
#include <string>

/**
 * @brief Static class for TCP network operations
 *
 * Provides methods to create sockets, accept connections,
 * send and receive data. Used for communication between
 * HOST and CLIENT modes.
 *
 * All methods are static, no instantiation is required.
 */
class Network {
public:
  /**
   * @brief Creates a TCP socket
   *
   * Creates a non-blocking socket for TCP communication.
   *
   * @return Socket descriptor or -1 in case of error
   */
  static int createSocket();

  /**
   * @brief Binds and sets socket to listen mode
   *
   * Associates the socket with a port and prepares to accept connections.
   * Used by the HOST to create the server.
   *
   * @param socket Socket descriptor
   * @param port TCP port to listen on (e.g., 8080)
   * @return true on success, false on error
   */
  static bool bindAndListen(int socket, int port);

  /**
   * @brief Accepts a client connection
   *
   * Accepts a pending connection (non-blocking).
   * Used by the HOST to accept the CLIENT.
   *
   * @param serverSocket Server socket descriptor
   * @return Client socket descriptor or -1 if no pending connection
   */
  static int acceptConnection(int serverSocket);

  /**
   * @brief Connects to a server
   *
   * Establishes a TCP connection with a server.
   * Used by the CLIENT to connect to the HOST.
   *
   * @param socket Socket descriptor
   * @param ip Server IP address (e.g., "127.0.0.1")
   * @param port Server port
   * @return true if connected, false on error
   */
  static bool connectToServer(int socket, const std::string &ip, int port);

  /**
   * @brief Sends data through the socket
   *
   * @param socket Socket descriptor
   * @param data Pointer to the data to send
   * @param size Number of bytes to send
   * @return true if sent, false on error
   */
  static bool sendData(int socket, const void *data, size_t size);

  /**
   * @brief Receives data from the socket
   *
   * Non-blocking operation. Returns immediately even
   * if no data is available.
   *
   * @param socket Socket descriptor
   * @param buffer Buffer to store received data
   * @param size Maximum buffer size
   * @return Number of bytes received, 0 if connection closed, -1 on error
   */
  static ssize_t receiveData(int socket, void *buffer, size_t size);

  /**
   * @brief Closes a socket
   *
   * Closes the socket and sets the variable to -1.
   *
   * @param socket Reference to the socket descriptor (modified to -1)
   */
  static void closeSocket(int &socket);
};

#endif // NETWORK_H
