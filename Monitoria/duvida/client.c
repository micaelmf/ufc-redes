#include<stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "packets.h"
#include "practical.h"
#include "framer.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {// Testa pelo número correto de argumentos
        DieWithUserMessage("Parametro(s)", "<Endereço Servidor/Nome> <Porta servidor/Serviço>");
    }

    char *server = argv[1];    // Primeiro arg: endereço servidor/nome
    char *service = argv[2];   // Segundo arg: porta/serviço

    //Passo 1 - Criar conexão com um servidor.
    int sock = SetupTCPClientSocket(server, service);
    if (sock < 0){
        DieWithUserMessage("SetupTCPClientSocket() falhou", "incapaz de conectar");
    }

    FILE *str = fdopen(sock, "r+"); // Envolve com um fluxo de E/S
    if (str == NULL){
        DieWithSystemMessage("fdopen() falhou");
    }

    //Passo 2 - Criar pacote do tipo 1 a ser enviado.
    Pacote* pacote = malloc(sizeof(Pacote));
    atribuiCabecalho(pacote);
    atribuiPayload1(pacote);

    //Passo 3 - Codifica para transmissão
    uint8_t outbuf[BUFSIZE];
    size_t reqSize = Encode(pacote, outbuf, BUFSIZE);

    // Enquadra e envia
    if(PutMsg(outbuf, reqSize, str) < 0){
        DieWithSystemMessage("PutMsg() falhou");
    }
    
    //Libera memória alocada pelo pacote
    liberaCabecalho(pacote);
    liberaPayload1(pacote);

    // Fecha
    fclose(str);
    
    exit(EXIT_SUCCESS);
}