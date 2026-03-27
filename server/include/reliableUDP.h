#ifndef RELIABLEUDP_H
#define RELIABLEUDP_H

#include "macros.h"

unsigned long calculateChecksum(const char *data, size_t length);
char *generate_shifted_base64_alphabet(const char *base64_chars, int shift);
long unsigned estimateTimeout(long unsigned *EstimatedRTT, long unsigned *DevRTT, long unsigned SampleRTT);

#endif
