#ifndef FRAMER_H_
#define FRAMER_H_

#include <stdio.h>
#include <inttypes.h>

int GetNextMsg(FILE *in, uint8_t *buf, size_t bufSize);
int PutMsg(uint8_t buf[], size_t msgSize, FILE *out);

#endif