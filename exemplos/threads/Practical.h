#ifndef PRACTICAL_H_
#define PRACTICAL_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>

// Manipula erro com mensagem do usuário
void DieWithUserMessage(const char *msg, const char *detail);
// Manipula erro com mensagem do sistema
void DieWithSystemMessage(const char *msg);
// Imprime endereço do socket
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);
// Testa igualidade do endereço do socket
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);
// Cria, associa e escuta um novo socket servidor TCP
int SetupTCPServerSocket(const char *service);
// Aceita uma nova conexão TCP no socket servidor
int AcceptTCPConnection(int servSock);
// Manipula novo cliente TCP
void HandleTCPClient(int clntSocket);
// Cria e conecta um novo socket cliente TCP
int SetupTCPClientSocket(const char *server, const char *service);

enum sizeConstants {
  MAXSTRINGLENGTH = 128,
  BUFSIZE = 512,
};

#endif // PRACTICAL_H_
