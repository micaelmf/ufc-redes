#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "Practical.h"

int main(int argc, char *argv[]) {

  if (argc != 3) // Testa o número de argumentos
    DieWithUserMessage("Parametro(s)", "<Endereço/Nome> <Porta/Serviço>");

  char *addrString = argv[1];   // Endereço/Nome do servidor
  char *portString = argv[2];   // Porta/serviço do servidor

  // Diz ao sistema que tipo de informações de endereço queremos
  struct addrinfo addrCriteria;                   // Critério para casamento de endereço
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zera estrutura
  addrCriteria.ai_family = AF_UNSPEC;             // Qualquer família de endereços
  addrCriteria.ai_socktype = SOCK_STREAM;         // Somente sockets de fluxo
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Somente protocolos TCP

  // Pega endereços associados com o nome/serviço especificado
  struct addrinfo *addrList; // Aponta para a lista de endereços retornados
  // Referencia a lista encadeada de endereços associados a um nome/serviço
  int rtnVal = getaddrinfo(addrString, portString, &addrCriteria, &addrList);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() falhou", gai_strerror(rtnVal));

  // Mostra os endereços retornados
  for (struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next) {
    PrintSocketAddress(addr->ai_addr, stdout);
    fputc('\n', stdout);
  }

  freeaddrinfo(addrList); // Libera a lista alocada por getaddrinfo()

  exit(0);
}
