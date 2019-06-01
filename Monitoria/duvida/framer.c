#include "framer.h"
#include "practical.h"
#include "packets.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>


/* Lê o cabeçalho. Então lê o número de dados indicados por payload_len.
 * Se o buffer de entrada é muito pequeno para o dado, trunca e
 * retorna a negação do comprimento indicado. Assim, um retorno negativo
 * que diferente de -1 indica que a mensagem foi truncada.
 * (Ambiguidade é possível somente se o método que chama passa um buffer vazio.)
 * Fluxo de entrada é sempre deixado vazio.
 */
int GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize) {
    uint32_t payload_len_n = 0;
    uint32_t payload_len_h = 0;
    uint32_t packet_len = 0;
    uint32_t extra = 0;

    //Lê o dado payload_len da cabeçalho do pacote que chega.
    if (fread(&payload_len_n, sizeof(uint32_t), 1, in) != 1){
        return -1;
    }

    // converteu para formato host
    payload_len_h = ntohl(payload_len_n);
    //tamanho total do pacote
    packet_len = payload_len_h + TAM_CABECALHO;

    if (packet_len > bufSize) {
        extra = packet_len - bufSize; //Armazena a quantidade de dados que sobram
        packet_len = bufSize; // Trunca
    }

    //Coloca o payload_len_n, que é o primeiro atributo do cabeçalho no buffer
    memcpy(buf, &payload_len_n, UI32SIZE); //Isto preencherá os primeiros 4 bytes.
    
    //coloca o restante do pacote (packet_len - sizeof(payload_len_n)) no buffer.
    uint32_t pkt_wout_paylen = (packet_len - sizeof(payload_len_n));

    if (fread(buf+4, sizeof(uint8_t), pkt_wout_paylen, in) != pkt_wout_paylen) {
        fprintf(stderr, "Erro enquadramento: esperado %d, leu menos\n", pkt_wout_paylen);
        return -1;
    }

    if (extra > 0) { // Mensagem foi trucada
        uint8_t waste[BUFSIZE];
        fread(waste, sizeof(uint8_t), extra, in); // Tenta limpar o canal
        return -(packet_len + extra); // Negação do tamanho indicado
    } else
        return packet_len;
}


/* Escreve a mensagem dada no fluxo de saída  
 * Pré-condição: buf[] é ao menos msgSize.
 * Retorna -1 em qualquer erro.
 */
int PutMsg(uint8_t buf[], size_t msgSize, FILE *out) {
    //se tamanho maior que o máximo permitido, retorna erro.
    if (msgSize > UINT16_MAX){
        return -1;
    }
    
    //se não enviou a mensagem toda para o socket, retorna erro.
    if ((fwrite(buf, sizeof(uint8_t), msgSize, out) != msgSize)){
        return -1;
    }
    
    //limpa o canal, ou seja, envia pela rede.
    fflush(out);
    
    return msgSize;
}