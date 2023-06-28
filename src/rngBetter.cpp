#include "rngBetter.h"
uint16_t __state = 0;
uint16_t xor16() {
  __asm__ __volatile__(
      "movw   r18, %A0"      "\n\t"
      "add    r18, r18"      "\n\t"
      "adc    r19, r19"      "\n\t"
      "add    r18, r18"      "\n\t"
      "adc    r19, r19"      "\n\t"
      "add    r18, r18"      "\n\t"
      "adc    r19, r19"      "\n\t"
      "eor    %A0, r18"      "\n\t"
      "eor    %B0, r19"      "\n\t"
      "mov     r18, %B0"     "\n\t"
      "swap    r18"          "\n\t"
      "lsr     r18"          "\n\t"
      "andi    r18, 0x07"    "\n\t"
      "eor     %A0, r18"     "\n\t"
      "mov    r19, %A0"      "\n\t"
      "add    r19, r19"      "\n\t"
      "eor    %B0, r19"      "\n\t"
  :"+d"((uint16_t)__state)::"r18","r19");
  return __state;
}
