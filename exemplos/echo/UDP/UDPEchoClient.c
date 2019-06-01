#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Practical.h"

int main(int argc, char *argv[]) {

  if (argc < 3 || argc > 4) // Verifica se tem o número correto de argumentos
    DieWithUserMessage("Parameter(s)",
        "<Server Address/Name> <Echo Word> [<Server Port/Service>]");

  char *server = argv[1];     // Primeiro arg: endereço servidor/nome
  char *echoString = argv[2]; // Segundo arg: palavra a ecoar

  size_t echoStringLen = strlen(echoString);
  if (echoStringLen > MAXSTRINGLENGTH) // Verifica tamanho da entrada
    DieWithUserMessage(echoString, "string too long");

  // Terceiro arg (opcional): porta servidor/serviço
  char *servPort = (argc == 4) ? argv[3] : "echo";

  // Diz ao sistema que tipos de endereços queremos
  struct addrinfo addrCriteria;                   // Critério para casamento de endereços
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Limpa a struct
  addrCriteria.ai_family = AF_UNSPEC;             // Qualquer família de endereços
  // Para os campos a seguir, zero significa que não se importa
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Somentes sockets datagrama
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Somente o protocolo UDP

  // Pega endereços
  struct addrinfo *servAddr; // Lista de endereços do servidor
  int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() falhou", gai_strerror(rtnVal));

  // Cria um socket datagram/UDP
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      servAddr->ai_protocol); // Descritor de socket para o cliente
  if (sock < 0)
    DieWithSystemMessage("socket() falhou");

  // Envia string ao servidor
  ssize_t numBytes = sendto(sock, echoString, echoStringLen, 0,
      servAddr->ai_addr, servAddr->ai_addrlen);
  if (numBytes < 0)
    DieWithSystemMessage("sendto() falhou");
  else if (numBytes != echoStringLen)
    DieWithUserMessage("sendto() error", "enviou número inexperado de bytes");

  freeaddrinfo(servAddr);

  close(sock);
  exit(0);
}
