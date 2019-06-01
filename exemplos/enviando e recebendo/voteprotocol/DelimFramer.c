#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Practical.h"

static const char DELIMITER = '\n';

/* Lê até bufSize bytes ou até delimitador, copiando no buffer disponibilizado.
 * Encontrar EOF depois de algum dado mas antes do delimitador resulta em falha.
 * (Isto é: EOF não é um delimitador válido.)
 * Retorna o número de bytes no buf (delimitador NÃO transferido).
 * Se buffer enche sem encontrar delimitador, contador negativo é retornado.
 * Se fluxo finaliza antes do primeiro byte, -1 é retornado.
 * Pré-condição: buf tem espaço para ao menos bufSize bytes.
 */
int GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize) {
  int count = 0;
  int nextChar;
  while (count < bufSize) {
    nextChar = getc(in);
    if (nextChar == EOF) {
      if (count > 0)
        DieWithUserMessage("GetNextMsg()", "Fluxo terminou prematuramente");
      else
        return -1;
    }
    if (nextChar == DELIMITER)
      break;
    buf[count++] = nextChar;
  }
  if (nextChar != DELIMITER) { // Fora do espaço: count==bufSize
    return -count;
  } else { // Encontrou delimitador
    return count;
  }
}

/* Escrever a mensagem ao fluxo de saída, seguida do delimitador.
 * Retorna o número de bytes lidos, ou -1 em falha.
 */
int PutMsg(uint8_t buf[], size_t msgSize, FILE *out) {
  // Verifica pelo delimitador na mensagem
  int i;
  for (i = 0; i < msgSize; i++)
    if (buf[i] == DELIMITER)
      return -1;
  if (fwrite(buf, 1, msgSize, out) != msgSize)
    return -1;
  fputc(DELIMITER, out);
  fflush(out);
  return msgSize;
}