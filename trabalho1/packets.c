#include "packets.h"
#include "practical.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

/*
    Objetivo: Duplicar uma string, retornado a cópia.

    src = Ponteiro p/ string que será duplicada.
    Retorna = ponteiro p/ a cópia da string.
*/
char *strdup(const char *src) {
    size_t len = strlen(src) + 1;       // String + '\0'
    char *dst = malloc(len);            // Aloca espaço
    if (dst == NULL) return NULL;       // Sem memória
    memcpy(dst, src, len);             // Copia o bloco
    return dst;                         // Retorna a nova string
}

void atribuiCabecalho(Pacote* pkt) {
    Cabecalho* hdr = malloc(sizeof(Cabecalho));

    hdr->payload_len = HELLO_SIZE;
    hdr->psecret = 10;
    hdr->step = 1;
    hdr->student_id = 365;

    pkt->cabecalho = hdr;
}

void alocaCabecalho(Pacote* pkt) {
    Cabecalho* hdr = malloc(sizeof(Cabecalho));
    pkt->cabecalho = hdr;
}

void atribuiPayload1(Pacote* pkt) {
    Payload_1* payload = malloc(sizeof(Payload_1));
    payload->msg = malloc(HELLO_SIZE);

    memcpy(payload->msg, HELLO_MSG, HELLO_SIZE);
    pkt->payload = payload;
}

void alocaPayload1(Pacote* pkt) {
    Payload_1* payload = malloc(sizeof(Payload_1));
    payload->msg = malloc(HELLO_SIZE);
    pkt->payload = payload;
}

void liberaPayload1(Pacote* pkt) {
    free(((Payload_1*)pkt->payload)->msg);
    free((Payload_1*)pkt->payload);
}

void liberaCabecalho(Pacote* pkt) {
    free(pkt->cabecalho);
}
/*
    Codifica o pacote no formato da rede.

    v = ponteiro p/ o pacote a ser codificado.
    outbuf = onde será armazenado o pacote no formato da rede.
    bufSize = tamanho do buffer.

    retorno = quantos bytes codificados.
*/
size_t Encode(Pacote *pkt, uint8_t *outBuf, size_t bufSize) {
    
    size_t size = 0;
    uint16_t u16 = 0;
    uint32_t u32 = 0;

    //Verifica se tem espaço no buffer para armazenar o pacote codificado.
    if(bufSize < sizeof(Pacote)) {
        DieWithUserMessage("Buffer muito pequeno", "");
    }

    //Limpa o buffer p/ ter certeza
    memset(outBuf, 0, bufSize);

    //converte payload_len e coloca no buffer.
    u32 = htonl(pkt->cabecalho->payload_len);
    memcpy(outBuf + PAYLOAD_LEN_SHIFT, &u32, UI32SIZE);
    size += UI32SIZE;
    //converte psecret e coloca no buffer.
    u32 = htonl(pkt->cabecalho->psecret);
    memcpy(outBuf + PSECRET_SHIFT, &u32, UI32SIZE);
    size += UI32SIZE;
    //converte step e coloca no buffer.
    u16 = htons(pkt->cabecalho->step);
    memcpy(outBuf + STEP_SHIFT, &u16, UI16SIZE);
    size += UI16SIZE;
    //converte student_id e coloca no buffer.
    u16 = htons(pkt->cabecalho->student_id);
    memcpy(outBuf + STUDENT_ID_SHIFT, &u16, UI16SIZE);
    size += UI16SIZE;
    //Coloca payload no buffer.
    memcpy(outBuf+size, ((Payload_1*)pkt->payload)->msg, HELLO_SIZE);
    size += HELLO_SIZE;

    return size;

}

bool Decode(uint8_t *inBuf, size_t mSize, Pacote *pkt){

    uint16_t u16 = 0;
    uint32_t u32 = 0;
    
    if(mSize < (TAM_CABECALHO + TAM_PAYLOAD_1)) {
        return false;
    }

    alocaCabecalho(pkt);
    alocaPayload1(pkt);

    //copia o tamanho do payload
    memcpy(&u32, inBuf + PAYLOAD_LEN_SHIFT, UI32SIZE);
    pkt->cabecalho->payload_len = ntohl(u32);
     
    //copia o psecret
    memcpy(&u32, inBuf + PSECRET_SHIFT, UI32SIZE);
    pkt->cabecalho->psecret = ntohl(u32);
 
    
    //copia o step
    memcpy(&u16, inBuf + STEP_SHIFT, UI16SIZE);
    pkt->cabecalho->step = ntohs(u16);
    
    //copia o student_id
    memcpy(&u16, inBuf + STUDENT_ID_SHIFT, UI16SIZE);
    pkt->cabecalho->student_id = ntohs(u16);
    
    //copia payload
    memcpy(((Payload_1*)pkt->payload)->msg, inBuf + TAM_CABECALHO, HELLO_SIZE);
 
    return true;
}
