#ifndef rngBetter_h
#define rngBetter_h
#include <rngUtil.h>

extern uint16_t __state;
uint16_t xor16();
inline bool seed_xor16(uint16_t seed) {
  if (seed) {
    __state = seed;
    return 1;
  }
  return 0;
}
inline uint16_t xor16(uint16_t seed) {
  if(__builtin_constant_p(seed)) {
    if(seed == 0) {
      badArg("Seed must not be zero");
      return 0;
    }
  } else if(seed == 0) {
    return 0;
  }
  seed_xor16(seed);
  return xor16();
}



/* And now for the complete list of full length RNGs. Someare bad. some are good. */
// Each function resides in it's own file along with it's seeding func, to ensure there isn't
// a risk of more than needed being included. I think LTO can still inline the seed function!
bool seed_xor16_11e(uint16_t seed);
bool seed_xor16_11f(uint16_t seed);
bool seed_xor16_152(uint16_t seed);
bool seed_xor16_174(uint16_t seed);
bool seed_xor16_17b(uint16_t seed);
bool seed_xor16_1b3(uint16_t seed);
bool seed_xor16_1f6(uint16_t seed);
bool seed_xor16_1f7(uint16_t seed);
bool seed_xor16_251(uint16_t seed);
bool seed_xor16_25d(uint16_t seed);
bool seed_xor16_25f(uint16_t seed);
bool seed_xor16_27d(uint16_t seed);
bool seed_xor16_27f(uint16_t seed);
bool seed_xor16_31c(uint16_t seed);
bool seed_xor16_31f(uint16_t seed);
bool seed_xor16_35b(uint16_t seed);
bool seed_xor16_3b1(uint16_t seed);
bool seed_xor16_3bb(uint16_t seed);
bool seed_xor16_3d9(uint16_t seed);
bool seed_xor16_437(uint16_t seed);
bool seed_xor16_471(uint16_t seed);
bool seed_xor16_4bb(uint16_t seed);
bool seed_xor16_57e(uint16_t seed);
bool seed_xor16_598(uint16_t seed);
bool seed_xor16_5b6(uint16_t seed);
bool seed_xor16_5bb(uint16_t seed);
bool seed_xor16_67d(uint16_t seed);
bool seed_xor16_6b5(uint16_t seed);
bool seed_xor16_6f1(uint16_t seed);
bool seed_xor16_71b(uint16_t seed);
bool seed_xor16_734(uint16_t seed);
bool seed_xor16_798(uint16_t seed);
bool seed_xor16_79d(uint16_t seed);
bool seed_xor16_7f1(uint16_t seed);
bool seed_xor16_895(uint16_t seed);
bool seed_xor16_897(uint16_t seed);
bool seed_xor16_97d(uint16_t seed);
bool seed_xor16_9d3(uint16_t seed);
bool seed_xor16_b17(uint16_t seed);
bool seed_xor16_b3d(uint16_t seed);
bool seed_xor16_b53(uint16_t seed);
bool seed_xor16_b71(uint16_t seed);
bool seed_xor16_bb3(uint16_t seed);
bool seed_xor16_bb4(uint16_t seed);
bool seed_xor16_bb5(uint16_t seed);
bool seed_xor16_c13(uint16_t seed);
bool seed_xor16_d3b(uint16_t seed);
bool seed_xor16_c3d(uint16_t seed);
bool seed_xor16_d3c(uint16_t seed);
bool seed_xor16_d52(uint16_t seed);
bool seed_xor16_d72(uint16_t seed);
bool seed_xor16_d76(uint16_t seed);
bool seed_xor16_d79(uint16_t seed);
bool seed_xor16_d97(uint16_t seed);
bool seed_xor16_e11(uint16_t seed);
bool seed_xor16_e75(uint16_t seed);
bool seed_xor16_f11(uint16_t seed);
bool seed_xor16_f13(uint16_t seed);
bool seed_xor16_f52(uint16_t seed);
bool seed_xor16_f72(uint16_t seed);

uint16_t xor16_11e();
uint16_t xor16_11f();
uint16_t xor16_152();
uint16_t xor16_174();
uint16_t xor16_17b();
uint16_t xor16_1b3();
uint16_t xor16_1f6();
uint16_t xor16_1f7();
uint16_t xor16_251();
uint16_t xor16_25d();
uint16_t xor16_25f();
uint16_t xor16_27d();
uint16_t xor16_27f();
uint16_t xor16_31c();
uint16_t xor16_31f();
uint16_t xor16_35b();
uint16_t xor16_3b1();
uint16_t xor16_3bb();
uint16_t xor16_3d9();
uint16_t xor16_437();
uint16_t xor16_471();
uint16_t xor16_4bb();
uint16_t xor16_57e();
uint16_t xor16_598();
uint16_t xor16_5b6();
uint16_t xor16_5bb();
uint16_t xor16_67d();
uint16_t xor16_6b5();
uint16_t xor16_6f1();
uint16_t xor16_71b();
uint16_t xor16_734();
uint16_t xor16_798();
uint16_t xor16_79d();
uint16_t xor16_7f1();
uint16_t xor16_895();
uint16_t xor16_897();
uint16_t xor16_97d();
uint16_t xor16_9d3();
uint16_t xor16_b17();
uint16_t xor16_b3d();
uint16_t xor16_b53();
uint16_t xor16_b71();
uint16_t xor16_bb3();
uint16_t xor16_bb4();
uint16_t xor16_bb5();
uint16_t xor16_c13();
uint16_t xor16_d3b();
uint16_t xor16_c3d();
uint16_t xor16_d3c();
uint16_t xor16_d52();
uint16_t xor16_d72();
uint16_t xor16_d76();
uint16_t xor16_d79();
uint16_t xor16_d97();
uint16_t xor16_e11();
uint16_t xor16_e75();
uint16_t xor16_f11();
uint16_t xor16_f13();
uint16_t xor16_f52();
uint16_t xor16_f72();


#endif
