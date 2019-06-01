#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "practical.h"
#include "packets.h"
#include "framer.h"

int main(int argc, char *argv[]) {
  if (argc != 2) // Testa pelo número correto de argumentos
    DieWithUserMessage("Parametro(s)", "<Porta servidor/Serviço>");

  int servSock = SetupTCPServerSocket(argv[1]);
  // servSock está pronto para aceitar conexões

  for (;;) { // Loop infinito
    // Espera por um cliente conectar
    int clntSock = AcceptTCPConnection(servSock);

    // Cria um fluxo de leitura com o socket
    FILE *channel = fdopen(clntSock, "r+");
    if (channel == NULL){
        DieWithSystemMessage("fdopen() falhou");
    }

    // Recebe mensagens até conexão fechar
    int mSize;
    uint8_t inBuf[BUFSIZE];
    Pacote* pacote = malloc(sizeof(Pacote));

    //Recebe uma mensagem pelo canal de fluxo de bytes
    while ((mSize = GetNextMsg(channel, inBuf, BUFSIZE)) > 0) {

      //Limpa a estrutura por garantia
      memset(pacote, 0, sizeof(Pacote));

      //Imprime tamanho da mensagem recebida

      if(Decode(inBuf, mSize, pacote)){
        //Imprime mensagem recebida
        printf("Msg no pacote 1: %s.\n", ((Payload_1*)pacote->payload)->msg);
      }

      //libera memória
      liberaCabecalho(pacote);
      liberaPayload1(pacote);

    }

    puts("Cliente finalizado");
    fclose(channel);
  }
  // Não chega aqui
}