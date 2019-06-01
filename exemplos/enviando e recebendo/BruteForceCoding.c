#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "Practical.h"

const uint8_t val8 = 101; // Cento e um
const uint16_t val16 = 10001; // Dez mil e um
const uint32_t val32 = 100000001; // Cem milhões e um
const uint64_t val64 = 1000000000001L; // Um trilhão e um
const int MESSAGELENGTH = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t)
    + sizeof(uint64_t);

static char stringBuf[BUFSIZE];
char *BytesToDecString(uint8_t *byteArray, int arrayLength) {
  char *cp = stringBuf;
  size_t bufSpaceLeft = BUFSIZE;
  for (int i = 0; i < arrayLength && bufSpaceLeft > 0; i++) {
    int strl = snprintf(cp, bufSpaceLeft, "%u ", byteArray[i]);
    bufSpaceLeft -= strl;
    cp += strl;
  }
  return stringBuf;
}

// Warning:  precondições não testadas (ex., 0 <= size <= 8)
int EncodeIntBigEndian(uint8_t dst[], uint64_t val, int offset, int size) {
  for (int i = 0; i < size; i++) {
    dst[offset++] = (uint8_t) (val >> ((size - 1) - i) * CHAR_BIT);
  }
  return offset;
}

// Warning:  precondições não testadas (ex., 0 <= size <= 8)
uint64_t DecodeIntBigEndian(uint8_t val[], int offset, int size) {
  uint64_t rtn = 0;
  for (int i = 0; i < size; i++) {
    rtn = (rtn << CHAR_BIT) | val[offset + i];
  }
  return rtn;
}

int main(int argc, char *argv[]) {
  uint8_t message[MESSAGELENGTH]; // Grande o suficiente para armazenar os quatro valores

  // Codifica inteiros em sequência no buffer de mensagem
  int offset = 0;
  offset = EncodeIntBigEndian(message, val8, offset, sizeof(uint8_t));
  offset = EncodeIntBigEndian(message, val16, offset, sizeof(uint16_t));
  offset = EncodeIntBigEndian(message, val32, offset, sizeof(uint32_t));
  offset = EncodeIntBigEndian(message, val64, offset, sizeof(uint64_t));
  printf("Mensagem codificada:\n%s\n", BytesToDecString(message, MESSAGELENGTH));

  uint64_t value =
      DecodeIntBigEndian(message, sizeof(uint8_t), sizeof(uint16_t));
  printf("Inteiro 2-bytes decodificado = %u\n", (unsigned int) value);
  value = DecodeIntBigEndian(message, sizeof(uint8_t) + sizeof(uint16_t)
      + sizeof(uint32_t), sizeof(uint64_t));
  printf("Inteiro 8-byte decodificado = %llu\n", value);

  // Mostra sinalização
  offset = 4;
  int iSize = sizeof(int32_t);
  value = DecodeIntBigEndian(message, offset, iSize);
  printf("Valor decodificado (offset %d, size %d) = %lld\n", offset, iSize, value);
  int signedVal = DecodeIntBigEndian(message, offset, iSize);
  printf("...se fosse valor com sinal %d\n", signedVal);
}