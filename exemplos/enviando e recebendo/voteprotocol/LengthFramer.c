#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include "Practical.h"

/* Lê 2 bytes de comprimento e coloca em ordem big-endian.
 * Então lê o número indicado de bytes.
 * Se o buffer de entrada é muito pequeno para o dado, trunca e
 * retorna a negação do comprimento indicado. Assim, um retorno negativo
 * que diferente de -1 indica que a mensagem foi truncada.
 * (Ambiguidade é possível somente se o método que chama passa um buffer vazio.)
 * Fluxo de entrada é sempre deixado vazio.
 */
int GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize) {
  uint16_t mSize = 0;
  uint16_t extra = 0;

  if (fread(&mSize, sizeof(uint16_t), 1, in) != 1)
    return -1;
  mSize = ntohs(mSize);
  if (mSize > bufSize) {
    extra = mSize - bufSize;
    mSize = bufSize; // Truncate
  }
  if (fread(buf, sizeof(uint8_t), mSize, in) != mSize) {
    fprintf(stderr, "Erro enquadramento: esperado %d, leu menos\n", mSize);
    return -1;
  }
  if (extra > 0) { // Mensagem foi trucada
    uint8_t waste[BUFSIZE];
    fread(waste, sizeof(uint8_t), extra, in); // Tenta limpar o canal
    return -(mSize + extra); // Negação do tamanho indicado
  } else
    return mSize;
}

/* Escreva a mensagem dada no fluxo de saída, seguida de
 * delimitador.  Pré-condição: buf[] é ao menos msgSize.
 * Retorna -1 em qualquer erro.
 */
int PutMsg(uint8_t buf[], size_t msgSize, FILE *out) {
  if (msgSize > UINT16_MAX)
    return -1;
  uint16_t payloadSize = htons(msgSize);
  if ((fwrite(&payloadSize, sizeof(uint16_t), 1, out) != 1) || (fwrite(buf,
      sizeof(uint8_t), msgSize, out) != msgSize))
    return -1;
  fflush(out);
  return msgSize;
}