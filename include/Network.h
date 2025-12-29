/**
 * @file Network.h
 * @brief Classe utilitária para comunicação de rede TCP
 * @author Projeto CG - Maze Game
 * @date 2025
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>
#include <string>

/**
 * @brief Classe estática para operações de rede TCP
 *
 * Fornece métodos para criar sockets, aceitar conexões,
 * enviar e receber dados. Usada para comunicação entre
 * o modo HOST e CLIENT.
 *
 * Todos os métodos são estáticos, não é necessário instanciar.
 */
class Network {
public:
  /**
   * @brief Cria um socket TCP
   *
   * Cria um socket não-bloqueante para comunicação TCP.
   *
   * @return Descritor do socket ou -1 em caso de erro
   */
  static int createSocket();

  /**
   * @brief Faz bind e coloca socket em modo listen
   *
   * Associa o socket a uma porta e prepara para aceitar conexões.
   * Usado pelo HOST para criar servidor.
   *
   * @param socket Descritor do socket
   * @param port Porta TCP para escutar (ex: 8080)
   * @return true se sucesso, false se erro
   */
  static bool bindAndListen(int socket, int port);

  /**
   * @brief Aceita uma conexão de cliente
   *
   * Aceita uma conexão pendente (não-bloqueante).
   * Usado pelo HOST para aceitar o CLIENT.
   *
   * @param serverSocket Descritor do socket servidor
   * @return Descritor do socket cliente ou -1 se nenhuma conexão pendente
   */
  static int acceptConnection(int serverSocket);

  /**
   * @brief Conecta a um servidor
   *
   * Estabelece conexão TCP com um servidor.
   * Usado pelo CLIENT para conectar ao HOST.
   *
   * @param socket Descritor do socket
   * @param ip Endereço IP do servidor (ex: "127.0.0.1")
   * @param port Porta do servidor
   * @return true se conectado, false se erro
   */
  static bool connectToServer(int socket, const std::string &ip, int port);

  /**
   * @brief Envia dados através do socket
   *
   * @param socket Descritor do socket
   * @param data Ponteiro para os dados a enviar
   * @param size Número de bytes a enviar
   * @return true se enviado, false se erro
   */
  static bool sendData(int socket, const void *data, size_t size);

  /**
   * @brief Recebe dados do socket
   *
   * Operação não-bloqueante. Retorna imediatamente mesmo
   * se não houver dados disponíveis.
   *
   * @param socket Descritor do socket
   * @param buffer Buffer para armazenar dados recebidos
   * @param size Tamanho máximo do buffer
   * @return Número de bytes recebidos, 0 se conexão fechada, -1 se erro
   */
  static ssize_t receiveData(int socket, void *buffer, size_t size);

  /**
   * @brief Fecha um socket
   *
   * Fecha o socket e define a variável como -1.
   *
   * @param socket Referência ao descritor do socket (modificado para -1)
   */
  static void closeSocket(int &socket);
};

#endif // NETWORK_H
