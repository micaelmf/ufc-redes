#include <limits.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("CHAR_BIT é %d\n\n",CHAR_BIT);         // Quantidade de bits em um char (normalmente 8!)

  printf("sizeof(char) é %d\n", sizeof(char));  // SEMPRE 1
  printf("sizeof(short) é %d\n", sizeof(short));
  printf("sizeof(int) é %d\n", sizeof(int));
  printf("sizeof(long) é %d\n", sizeof(long));
  printf("sizeof(long long) é %d\n\n", sizeof(long long));

  printf("sizeof(int8_t) é %d\n", sizeof(int8_t));
  printf("sizeof(int16_t) é %d\n", sizeof(int16_t));
  printf("sizeof(int32_t) é %d\n", sizeof(int32_t));
  printf("sizeof(int64_t) é %d\n\n", sizeof(int64_t));

  printf("sizeof(uint8_t) é %d\n", sizeof(uint8_t));
  printf("sizeof(uint16_t) é %d\n", sizeof(uint16_t));
  printf("sizeof(uint32_t) é %d\n", sizeof(uint32_t));
  printf("sizeof(uint64_t) é %d\n", sizeof(uint64_t));
}