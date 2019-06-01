#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "practical.h"

static const int MAXPENDING = 5; // Máximo de requisições pendentes

int SetupTCPServerSocket(const char *service) {
  // Constrói a estrutura de endereços do servidor
  struct addrinfo addrCriteria;                   // Critério para correspondência de endereço
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Limpa a struct
  addrCriteria.ai_family = AF_UNSPEC;             // Qualquer família de endereços
  addrCriteria.ai_flags = AI_PASSIVE;             // Aceita em qualquer porta
  addrCriteria.ai_socktype = SOCK_STREAM;         // Somente socket de fluxo
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Somente o protocolo TCP

  struct addrinfo *servAddr; // Lista de endereços do servidor
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() falhou", gai_strerror(rtnVal));

  int servSock = -1;
  for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Cria socjet TCP
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    if (servSock < 0)
      continue;       // Criação do socket falhou; tente com outro endereço

    // Associe a um endereço local e defina que o socket irá escutar
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
      break;       // Bind e listen com sucesso
    }

    close(servSock);  // Fecha e tenta novamente
    servSock = -1;
  }

  // Libera a memória da lista de endereços alocada por getaddrinfo()
  freeaddrinfo(servAddr);

  return servSock;
}

int AcceptTCPConnection(int servSock) {
  struct sockaddr_storage clntAddr; // Endereço cliente
  // Calcula o tamanho da estrutura de endeço do cliente
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Espera connect do cliente
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() falhou");

  // clntSock está conectado a um cleinte

  fputs("Manipulando cliente ", stdout);
  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);

  return clntSock;
}

void HandleTCPClient(int clntSocket) {
  char buffer[BUFSIZE]; // Buffer para string echo

  // Recebe mensagem do cliente
  ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
  if (numBytesRcvd < 0)
    DieWithSystemMessage("recv() falhou");

  // Envia a string recebida
  while (numBytesRcvd > 0) { // 0 indica fim do fluxo
    // Mensagem enviada ao cliente
    ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
    if (numBytesSent < 0)
      DieWithSystemMessage("send() falhou");
    else if (numBytesSent != numBytesRcvd)
      DieWithUserMessage("send()", "enviou número inesperado de bytes");

    // Vê se há mais bytes para receber
    numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
    if (numBytesRcvd < 0)
      DieWithSystemMessage("recv() failed");
  }

  close(clntSocket); // Fecha socket do cliente
}
