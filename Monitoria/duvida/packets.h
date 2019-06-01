#ifndef PACKETS_H_
#define PACKETS_H_

//Inclusões de dependências
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

//Macros, enum e constantes globais
#define HELLO_MSG "Hello World"
#define HELLO_SIZE 12
#define UI32SIZE 4
#define UI16SIZE 2

enum {
    PAYLOAD_LEN_SHIFT = 0,
    PSECRET_SHIFT = 4,
    STEP_SHIFT = 8,
    STUDENT_ID_SHIFT = 10,
    TAM_CABECALHO = 12,
    TAM_PAYLOAD_1 = 12,
};

//Pacotes
struct header
{
    uint32_t payload_len;
    uint32_t psecret;
    uint16_t step;
    uint16_t student_id;
};

typedef struct header Cabecalho;

struct payload_1
{
    char *msg;
};

typedef struct payload_1 Payload_1;

struct packet
{
    Cabecalho* cabecalho;
    void *payload;
};

typedef struct packet Pacote;

//Facilitadores de manipulação
void atribuiCabecalho(Pacote*);
void atribuiPayload1(Pacote*);
void alocaCabecalho(Pacote*);
void alocaPayload1(Pacote*);
void liberaPayload1(Pacote*);
void liberaCabecalho(Pacote*);

bool Decode(uint8_t *inBuf, size_t mSize, Pacote *v);
size_t Encode(Pacote *v, uint8_t *outBuf, size_t bufSize);

#endif