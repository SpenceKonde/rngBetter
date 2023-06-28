// XOR 16 // full period generator: d79 //

#include "rng16compl.h"
static uint16_t __xor16_d79_state;

bool seed_xor16_d79(uint16_t seed) {
  if (__builtin_constant_p(seed)) {
    if(seed == 0) {
      badArg("rng's must be seeded with a non-zero value, but the argument passed is always zero");
    }
  }
  if(seed) {
    __xor16_d79_state = seed;
    return 1;
  }
  return 0;
}

uint16_t xor16_d79() {
    __asm__ __volatile__(
      "mov    r19, %A0"      "\n\t"
      "swap   r19"           "\n\t"
      "add    r19, r19"      "\n\t"
      "andi   r19, 0xE0"     "\n\t"
      "eor    %B0, r19"      "\n\t"
      "movw   r18, %A0"      "\n\t"
      "add    r18, r18"      "\n\t"
      "mov    r18, r19"      "\n\t"
      "adc    r18, r18"      "\n\t"
      "sbc    r19, r19"      "\n\t"
      "neg    r19"           "\n\t"
      "eor    %A0, r18"      "\n\t"
      "eor    %B0, r19"      "\n\t"
      "mov    r19, %A0"      "\n\t"
      "add    r19, r19"      "\n\t"
      "eor    %B0, r19"      "\n\t"
    :"+d"((uint16_t)__xor16_d79_state)::r"r18","r19");
  return __xor16_d79_state;
}
