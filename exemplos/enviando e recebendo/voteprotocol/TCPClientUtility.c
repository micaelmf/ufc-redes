#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Practical.h"

int SetupTCPClientSocket(const char *host, const char *service) {
  // Diz ao sistema que tipos de endereços queremos
  struct addrinfo addrCriteria;                   // Critério para filtro de endereços
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Limpa a estrutura
  addrCriteria.ai_family = AF_UNSPEC;             // v4 ou v6 está OK
  addrCriteria.ai_socktype = SOCK_STREAM;         // Somente sockets de fluxo
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Somente protocolos TCP

  // Pega endereços
  struct addrinfo *servAddr; // Aponta para a lista retornada com os endereços do servidor
  int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  int sock = -1;
  for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Cria um socket de fluxo, confiável, TCP
    sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sock < 0)
      continue;  // Criação de socket falhou; tenta um próximo endereço

    // Estabelece conexão ao servidor echo
    if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
      break;     // Conexão do Socket com sucesso; para e retorna o socket

    close(sock); // Conexão do Socket falhou; tentam o próximo endereço
    sock = -1;
  }

  freeaddrinfo(servAddr); // Libera a lista de addrinfo alocada em getaddrinfo()
  return sock;
}
