#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "Practical.h"
#include "VoteProtocol.h"
#include "VoteEncoding.h"
#include "Framer.h"

static uint64_t counts[MAX_CANDIDATE + 1];

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
    if (channel == NULL)
      DieWithSystemMessage("fdopen() falhou");

    // Recebe mensagens até conexão fechar
    int mSize;
    uint8_t inBuf[MAX_WIRE_SIZE];
    VoteInfo v;
    while ((mSize = GetNextMsg(channel, inBuf, MAX_WIRE_SIZE)) > 0) {
      memset(&v, 0, sizeof(v)); // Limpa informação de voto
      printf("Mensagem recebida (%d bytes)\n", mSize);
      if (Decode(inBuf, mSize, &v)) { // Decodifica para VoteInfo
        if (!v.isResponse) { // Ignora o que não for requisição
          v.isResponse = true;
          if (v.candidate >= 0 && v.candidate <= MAX_CANDIDATE) {
            if (!v.isInquiry)
              counts[v.candidate] += 1;
            v.count = counts[v.candidate];
          } // Ignora candidatos inválidos
        }
        uint8_t outBuf[MAX_WIRE_SIZE];
        mSize = Encode(&v, outBuf, MAX_WIRE_SIZE);
        if (PutMsg(outBuf, mSize, channel) < 0) {
          fputs("Erro de messagem\n", stderr);
          break;
        } else {
          printf("Processado %s para o candidato %d; votos atuais: %llu.\n",
              (v.isInquiry ? "inquiry" : "vote"), v.candidate, v.count);
        }
        fflush(channel);
      } else {
        fputs("Erro de decodificação, fechando conexão.\n", stderr);
        break;
      }
    }
    puts("Cliente finalizado");
    fclose(channel);
  } // Cada cliente
  // Não chega aqui
}