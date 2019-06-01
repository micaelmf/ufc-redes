#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"

static const int MAXPENDING = 5; // Número máximo de conexões pendentes

int main(int argc, char *argv[]) {

  if (argc != 2) // Testa para número correto de argumentos
    DieWithUserMessage("Parametros(s)", "<Porta servidor>");

  in_port_t servPort = atoi(argv[1]); // Primeiro argumento:  porta local

  // Cria socket para as requisições de conexões que chegarem
  int servSock; // Descritor de socket para o servidor
  if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) // o parâmetro protocolo, o terceiro, poderia ser 0. TCP seria escolhido por ser o padrão.
    DieWithSystemMessage("socket() falhou");

  // Constrói struct de endereço local
  struct sockaddr_in servAddr;                  // Endereço local
  memset(&servAddr, 0, sizeof(servAddr));       // Limpa a struct
  servAddr.sin_family = AF_INET;                // Família de endereço IPv4
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // De qualquer interface
  servAddr.sin_port = htons(servPort);          // Porta local

  // Associa ao endereço local
  if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    DieWithSystemMessage("bind() falhou");

  // Marca o socket que irá escutar as requisições de conexão
  if (listen(servSock, MAXPENDING) < 0)
    DieWithSystemMessage("listen() falhou");

  for (;;) { // Executa para sempre
    struct sockaddr_in clntAddr; // Endereço cliente
    // Define o tamanho da struct de endereço do cliente
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Espera pela conexão de um cliente
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0)
      DieWithSystemMessage("accept() falhou");

    // clntSock está conectado a um cliente!

    char clntName[INET_ADDRSTRLEN]; // String que conterá o endereço do cliente
    if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
        sizeof(clntName)) != NULL)
      printf("Manipulando cliente %s/%d\n", clntName, ntohs(clntAddr.sin_port));
    else
      puts("Incapaz de obter endereço do cliente");

    HandleTCPClient(clntSock);
  }
  // Não alcançado
}
