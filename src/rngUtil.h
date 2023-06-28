/* Never include this directly */
#ifndef rngTools_h
#define rngTools_h
namespace rng16 {
  uint16_t ADCtoSeed(uint16_t reading, uint8_t length) {
    uint16_t tempseed;
    switch (length) {
      case 8:
        __asm__ __volatile__(
          "mov %A1, %B0"     "\n\t"
          "mov %A0, %A1"     "\n\t"
          "com %A1"          "\n\t"
        :"+d"(uint16_t)tempseed:"r"(uint16_t)reading:);
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
        :"+d"(uint16_t)tempseed:"r"(uint16_t)reading:);
        break;
      case 12:
      case 13:
        __asm__ __volatile__(
          "mov %A1, %B0"      "\n\t"
          "mov r0, %B0"       "\n\t"
          "mov %A0, %B1"      "\n\t"
          "eor %A0, r0"       "\n\t"
        :"+d"(uint16_t)tempseed:"r"(uint16_t)reading:);
        break;
      case 14:
        __asm__ __volatile__(
          "mov %A1, %B0"     "\n\t"
          "mov r0, %B0"      "\n\t"
          "mov %A0, %B1"     "\n\t"
          "swap %A0"         "\n\t"
          "com r0"           "\n\t"
          "eor %A0, r0"      "\n\t"
        :"+d"(uint16_t)tempseed:"r"(uint16_t)reading:);
        break;
      case 15:
        __asm__ __volatile__(
          "movw %A0, %B0"    "\n\t"
          "swap %A0"         "\n\t"
          "swap %A1"         "\n\t" //Now that zero is in bit 11
          "bst %B0, 7"       "\n\t" //store a bit that's not necessarily a 0.
          "bld %A1, 3"       "\n\t" // and load it to the 0 bit.
          "eor %A0, %B1"     "\n\t"
          "eor %A1, %B0"     "\n\t"
        :"+d"(uint16_t)tempseed:"r"(uint16_t)reading:);
        break;
      case 16:
        __asm__ __volatile__(
          "movw %A0, %B0"    "\n\t"
          "swap %A0"         "\n\t"
          "swap %A1"         "\n\t"
          "eor %A0, %B1"     "\n\t"
          "eor %A1, %B0"     "\n\t"
        :"+d"(uint16_t)tempseed:"r"(uint16_t)reading:);
        break;
      default:
        return 0;
        break;

    }
    return tempseed;
  }
  uint16_t timeADCtoSeed(uint16_t reading, uint8_t length, uint16_t tickcount) {
    uint8_t tempval;
    __asm__ __volatile__( // scramble up the tick count - swap the bytes, swap each nybble, then finally eor it with 0b10100101 so on average it should have equal 1's and 0s.
      "mov r0, %A0"     "\n\t"
      "mov %A0 %A1"     "\n\t"
      "mov %A1, r0"     "\n\t"
      "ldi %B0, 0x55"   "\n\t"
      "swap %A0"        "\n\t"
      "swap %A1"        "\n\t"
      "eor %A0, %B0"    "\n\t"
      "com %B0"         "\n\t"
      "eor %A1, %B0"    "\n\t"
    :"+r"(uint16_t) tickcount, "+d" (uint8_t) tempval);
    tempseed = ADCtoSeed(uint16_t reading, uint8_t length);
    return tempseed^tickcount;
  }
}
