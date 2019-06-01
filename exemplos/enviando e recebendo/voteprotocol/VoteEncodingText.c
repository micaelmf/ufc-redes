/* Rotinas para codificação do texto da mensagem.
 * Formato:
 *   "Voting <v|i> [R]  <ID candidato>  <quantidade>"
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Practical.h"
#include "VoteProtocol.h"

static const char *MAGIC = "Voting";
static const char *VOTESTR = "v";
static const char *INQSTR = "i";
static const char *RESPONSESTR = "R";
static const char *DELIMSTR = " ";
enum {
  BASE = 10
};

/* Codifica a mensagem como texto.
 * WARNING: Mensagem será truncada se buffer pequeno!
 * Invariantes (ex. 0 <= candidato <= 1000) não verificadas.
 */
size_t Encode(const VoteInfo *v, uint8_t *outBuf, const size_t bufSize) {
  uint8_t *bufPtr = outBuf;
  long size = (size_t) bufSize;
  int rv = snprintf((char *) bufPtr, size, "%s %c %s %d", MAGIC,
      (v->isInquiry ? 'i' : 'v'), (v->isResponse ? "R" : ""), v->candidate);
  bufPtr += rv;
  size -= rv;
  if (v->isResponse) {
    rv = snprintf((char *) bufPtr, size, " %llu", v->count);
    bufPtr += rv;
  }
  return (size_t) (bufPtr - outBuf);
}

/* Extrai informação da mensagem de um buffer.
 * Nota: modifica buffer de entrada
 */
bool Decode(uint8_t *inBuf, const size_t mSize, VoteInfo *v) {

  char *token;
  token = strtok((char *) inBuf, DELIMSTR);
  // Verifica código mágico
  if (token == NULL || strcmp(token, MAGIC) != 0)
    return false;

  // Pega indicador de vote/inquiry
  token = strtok(NULL, DELIMSTR);
  if (token == NULL)
    return false;

  if (strcmp(token, VOTESTR) == 0)
    v->isInquiry = false;
  else if (strcmp(token, INQSTR) == 0)
    v->isInquiry = true;
  else
    return false;

  // Próximo token é uma flag Resposta ou ID do candidato
  token = strtok(NULL, DELIMSTR);
  if (token == NULL)
    return false; // Mensagem muito curta

  if (strcmp(token, RESPONSESTR) == 0) { // Flag de resposta presente
    v->isResponse = true;
    token = strtok(NULL, DELIMSTR); // Pega ID do candidato
    if (token == NULL)
      return false;
  } else { // Sem flag de resposta; token é o ID do candidato;
    v->isResponse = false;
  }
  // Pega candidato #
  v->candidate = atoi(token);
  if (v->isResponse) { // Mensagem de resposta deve conter valor da quantidade
    token = strtok(NULL, DELIMSTR);
    if (token == NULL)
      return false;
    v->count = strtoll(token, NULL, BASE);
  } else {
    v->count = 0L;
  }
  return true;
}