#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "Practical.h"
#include "VoteProtocol.h"
#include "Framer.h"
#include "VoteEncoding.h"

int main(int argc, char *argv[]) {
  if (argc < 4 || argc > 5)  // Testa o número correto de args
    DieWithUserMessage("Parametro(s)", "<Endereço Servidor/Nome> <Porta Servidor/Serviço> <Candidato> [I]");

  char *server = argv[1];    // Primeiro arg: endereço servidor/nome
  char *service = argv[2];   // Segundo arg: porta/serviço
  int candi = atoi(argv[3]); // Terceiro arg: ID candidato
  if (candi < 0 || candi > MAX_CANDIDATE)
    DieWithUserMessage("Candidato # inválido", argv[3]);

  bool inq = argc > 4 && strcmp(argv[4], "I") == 0;

  // Cria um socket TCP conectado ao servidor
  int sock = SetupTCPClientSocket(server, service);
  if (sock < 0)
    DieWithUserMessage("SetupTCPClientSocket() falhou", "incapaz de conectar");

  FILE *str = fdopen(sock, "r+"); // Envolve com um fluxo de E/S
  if (str == NULL)
    DieWithSystemMessage("fdopen() falhou");

  // Constrói a informação para a requisição
  VoteInfo vi;
  memset(&vi, 0, sizeof(vi));

  vi.isInquiry = inq;
  vi.candidate = candi;

  // Codifica para a transmissão
  uint8_t outbuf[MAX_WIRE_SIZE];
  size_t reqSize = Encode(&vi, outbuf, MAX_WIRE_SIZE);

  // Imprime informação
  printf("Enviando %d-byte %s para o candidato %d...\n", reqSize,
	 (inq ? "inquiry" : "vote"), candi);

  // Enquadra e envia
  if (PutMsg(outbuf, reqSize, str) < 0)
    DieWithSystemMessage("PutMsg() falhou");

  // Recebe e imprime a resposta
  uint8_t inbuf[MAX_WIRE_SIZE];
  size_t respSize = GetNextMsg(str, inbuf, MAX_WIRE_SIZE); // Pega a mensagem
  if (Decode(inbuf, respSize, &vi)) { // Decodifica
    printf("Recebeu:\n");
    if (vi.isResponse)
      printf("  Resposta à ");
    if (vi.isInquiry)
      printf("inquiry ");
    else
      printf("vote ");
    printf("para o candidato %d\n", vi.candidate);
    if (vi.isResponse)
      printf("  quantidade = %llu\n", vi.count);
  }

  // Fecha
  fclose(str);

  exit(0);
}