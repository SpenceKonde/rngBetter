/* Never include this directly */
#ifndef rngTools_h
#define rngTools_h
#include <Arduino.h>
#include <stdint.h>
namespace rng16 {
  uint16_t ADCtoSeed(uint16_t reading, uint8_t length);
  uint16_t timeADCtoSeed(uint16_t reading, uint8_t length, uint16_t tickcount);
}
#endif
