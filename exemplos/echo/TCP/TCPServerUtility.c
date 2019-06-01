#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "Practical.h"

static const int MAXPENDING = 5; // Número máximo de conexões pendentes

int SetupTCPServerSocket(const char *service) {
  // Constrói a struct de endereço do servidor
  struct addrinfo addrCriteria;                   // Critério para casamento de endereço
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Limpa a struct
  addrCriteria.ai_family = AF_UNSPEC;             // Qualquer família de endereços
  addrCriteria.ai_flags = AI_PASSIVE;             // Aceita em qualquer endereço/porta
  addrCriteria.ai_socktype = SOCK_STREAM;         // Somente sockets de fluxo
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Somente protocolo TCP

  struct addrinfo *servAddr; // Lista de endereços servidor
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() falhou", gai_strerror(rtnVal));

  int servSock = -1;
  for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Cria um socket TCP
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    if (servSock < 0)
      continue;       // Criação do socket falhou; tenta o próximo endereço

    // Associa ao endereço local e configura o socket para escutar
    if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
        (listen(servSock, MAXPENDING) == 0)) {
      // Imprime o endereço local do socket
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);
      if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
        DieWithSystemMessage("getsockname() falhou");
      fputs("Associado a ", stdout);
      PrintSocketAddress((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);
      break;       // Associação e configuração para escutar realizadas com sucesso
    }

    close(servSock);  // Fecha e tenta novamente
    servSock = -1;
  }

  // Libera memória da lista de endereços alocada por getaddrinfo()
  freeaddrinfo(servAddr);

  return servSock;
}

int AcceptTCPConnection(int servSock) {
  struct sockaddr_storage clntAddr; // Endereço cliente
  // Define o tamanho da struc de endereço do cliente
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Espera pela conexão de um cliente
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() failed");

  // clntSock está conectado a um cliente

  fputs("Handling client ", stdout);
  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);

  return clntSock;
}

void HandleTCPClient(int clntSocket) {
  char buffer[BUFSIZE]; // Buffer para o eco da string

  // Recebe mensagem do cliente
  ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
  if (numBytesRcvd < 0)
    DieWithSystemMessage("recv() falhou");

  // Envia a string recebida e recebe novamente até o final do fluxo
  while (numBytesRcvd > 0) { // 0 indica fim do fluxo
    // Echo message back to client
    ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
    if (numBytesSent < 0)
      DieWithSystemMessage("send() falhou");
    else if (numBytesSent != numBytesRcvd)
      DieWithUserMessage("send()", "enviou número inesperado de bytes");

    // Vê se há mais bytes para receber
    numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
    if (numBytesRcvd < 0)
      DieWithSystemMessage("recv() falhou");
  }

  close(clntSocket); // Fecha socket cliente
}
