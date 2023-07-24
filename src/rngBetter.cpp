#include <rngBetter.h>
uint16_t rng16::ADCtoSeed(uint16_t reading, uint8_t length) {
    uint16_t tempseed = 1;
    switch (length) {
      case 8:
        __asm__ __volatile__(
          "mov %A1, %B0"     "\n\t"
          "mov %A0, %A1"     "\n\t"
          "com %A1"          "\n\t"
        :"+d"((uint16_t)tempseed)
        :"r"((uint16_t)reading)
        :);
        break;
      case 10:
      /* 0b000000ab cdefghij
       *   cdefghij 000000ab   ghijcdef
       *
       */
        __asm__ __volatile__(
          "mov  r0, %B0"     "\n\t"
          "mov %A0, %B1"     "\n\t"
          "mov %A1,  r0"     "\n\t"
          "swap r0     "     "\n\t"
          "eor %A0,  r0"     "\n\t"
          "mov  r0, %B1"     "\n\t"
          "mov %A0,  r0"     "\n\t"
          "swap r0     "     "\n\t"
          "eor %A1,  r0"     "\n\t"
        :"+d"((uint16_t)tempseed)
        :"r"((uint16_t)reading)
        :);
        break;
      case 12:
      case 13:
        __asm__ __volatile__(
          "mov %A1, %B0"      "\n\t"
          "mov r0, %B0"       "\n\t"
          "mov %A0, %B1"      "\n\t"
          "eor %A0, r0"       "\n\t"
        :"+d"((uint16_t)tempseed)
        :"r"((uint16_t)reading)
        :);
        break;
      case 14:
        __asm__ __volatile__(
          "mov %A1, %B0"     "\n\t"
          "mov r0, %B0"      "\n\t"
          "mov %A0, %B1"     "\n\t"
          "swap %A0"         "\n\t"
          "com r0"           "\n\t"
          "eor %A0, r0"      "\n\t"
        :"+d"((uint16_t)tempseed)
        :"r"((uint16_t)reading)
        :);
        break;
      case 15:
        __asm__ __volatile__(
          "movw %A0, %A1"    "\n\t"
          "swap %A0"         "\n\t"
          "swap %B1"         "\n\t" //Now that zero is in bit 11
          "bst %A1, 7"       "\n\t" //store a bit that's not necessarily a 0.
          "bld %B0, 3"       "\n\t" // and load it to the 0 bit.
          "eor %A0, %B1"     "\n\t"
          "eor %B0, %A1"     "\n\t"
        :"+d"((uint16_t)tempseed)
        :"r"((uint16_t)reading)
        :);
        break;
      case 16:
        __asm__ __volatile__(
          "movw %A0, %A1"    "\n\t"
          "swap %A0"         "\n\t"
          "swap %B1"         "\n\t"
          "eor %A0, %B1"     "\n\t"
          "eor %B0, %A1"     "\n\t"
        :"+d"((uint16_t)tempseed)
        :"r"((uint16_t)reading)
        :);
        break;
      default:
        return 0;
        break;

    }
    return tempseed;
  }
  uint16_t rng16::timeADCtoSeed(uint16_t reading, uint8_t length, uint16_t tickcount) {
    uint8_t tempval = 0;
    __asm__ __volatile__( // scramble up the tick count - swap the bytes, swap each nybble, then finally eor it with 0b10100101 so on average it should have equal 1's and 0s.
      "mov   r0,  %A0"    "\n\t"
      "mov  %A0,  %A1"    "\n\t"
      "mov  %A1,   r0"    "\n\t"
      "ldi  %B0, 0x55"    "\n\t"
      "swap %A0"          "\n\t"
      "swap %A1"          "\n\t"
      "eor  %A0,  %B0"    "\n\t"
      "com  %B0"          "\n\t"
      "eor  %A1,  %B0"    "\n\t"
    :"+r"((uint16_t) tickcount),
     "+d"((uint8_t) tempval)
    ::);
    uint16_t tempseed = ADCtoSeed(reading, length);
    return tempseed ^ tickcount;
  }
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
