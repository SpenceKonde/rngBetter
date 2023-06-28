# rngBetter
 A high performance, non-cryptographically secure PRNG for Arduino

This is an attempt to implement performant, space efficient random number generation for an AVR device in an Arduino or non-Arduino context. This is achieved with the XORSHIFT scheme, well known for being fast to implement in limited resource environment. These are implemented in hand-optimized assembly, because avr-gcc does a pretty miserable job of optimizing it from C code. The application of this library is intended to be where large numbers of random numbers are required, for example when using the device to control an apparently non-deterministic pattern on an array of LEDs (for a conceptually simplified version of this, imagine several hundred LED arranged in an arbitrary manner across a surface. Illumination of an experimentally determined brightness was desired, giving a red, green and blue value that, when assigned to every LED provides the desired illumination. Such a device would be an odd looking, panel-like lamp, but it would not be visually interesting, so it is desired to make each LED's color and brightness vary around that setpoint at random. So for each color you might want to say that there was a 50% chance of remaining the same as the previous frame, otherwise with equal likelihood it would get a bit brighter or dimmer, varying within a bounded, configurable range. This requires 600 2-bit random numbers per frame - and many implementations might want more or more granular values in order to have a smoother behavior near the boundary to bias it towards moving towards the setpoint, or to perform the calculation considering the entire LED). Regardless of the specifics, you need a lot of random numbers, you probably don't need very many bits each, you don't care *that* much about the quality of random numbers, but - as you'll be bitslicing the values - you really don't want to get a bunch of consecutive values with bits not changing.

In the above example, for frame rate f, f = 1/t<sub>frame</sub>. t<sub>frame</sub> = N * t<sub>pixel</sub>. Say we specify that the frame rate is required to be at minimum 30 fps to appear smooth, and ask how many LEDs the controller can handle. We get N = 1/30t<sub>pixel</sub>. Neglecting the calculation time, this yields approximately 1000 LEDs. But if it took us as long to calculate that as it did to send it, we can only control half as many LEDs. If there are 1,000 LEDs to control, a controller that took that long to drive them would be able to control 500 LEDs - we need 2 controllers, and mechanisms to provide any needed synchronization. If it took 4 times as long instead, we'd need 5 controllers, each driving only 200 LEDs. Plus a means tosynchropnize them all which would mean betweemn 1 and 3 additional devices. So for the intended use cases, the performance of the animation algorithm, including the random generator will determine how many processors are needed to meet the desired system specifications, and hence the cost of the whole thing. Of course, once you have a thousand leds attached to a panel, and then realized that 96% of the CPU time would be spent sending data, leaving 4% for your animation algorithm, you suddenly find yourself up a creek without a paddle,

A system where multiple controllers are used will, of course, often involve some coordination overhead as well. Perhaps they could all have their led control patterns setup and that'd be the end of it, and the closest thing to synchronization would be turning on the powerstrip running them all.

If they're too far for that? Some 433 MHz OOK outlets might work a treat.

But many times, you need much richer controls than that. You want it to look like one string, or to have complicated synchronized behaviors.
The complexity of these systems rises rapidly, and can quickly impose enough overhead that each controller needs a separate synchronization controller (because the sync isdont over OOK RF or something, and you cant do that if yo're spending all your timre outputting LED patterns)

## So what's wrong with random() or math.random()?
In a word - performance. Or more explicitly, the lack thereof. The arduino random function is simply based on the standard library's implementation. You'd think they'd have put some thought into the performance, but that doesn't seem to have been the case. 6/7ths or so of the time it's in that routine **it is in the process of a 32-bit divion operation** which uses around SIX HUNDRED CLOCKS. Division on any platform without hardware division operator is already pretty miserable - doing it when you can only operate one one byte at a time makes it much worse. Converting this value into a value within a specified range is horribly inefficient as well; the latter part was written by the Arduino team, so it's no surprise that it is not performant either. I'm sort of surprised by how spectacularly bad the avr-libc version is though - an xorshift32 would have blown it out of the water, or if they thought that performance was acceptable as is, could have been a better RNG.

## Okay, so why'd you have to do it all in ASM?
We didn't have to do it in ASM. But we wanted it to be performant. There are two enemies here that we must get down onto the bare metal to fight with hope of success. First, the C types can have unexpected effects, resulting from unexpected type probmotion which hurts performance significantly
These are in many cases motivated by performance issues with the `-Os` optimization - that flag tells the compiler "optimize for size above all else", which is what you want to do most of the time on a microcontroller. But occasionally, it's very much not what you want - consider the case of shifting a `uint16_t x` left a constant number of places - `x << n` for constant `16 > n > 0`. This normally generates at most 10 bytes of code (not counting whatever you do with the result or whether you have to copy the starting value, which you usually do), but the worst case of `n = 6` takes 30 clock cycles to execute; if optimized for speed, that's still the worst case, but it only takes 12 clock cycles, for a cost of 14 additional bytes. If this function happens to be time-critical, that 30 clock cycle runtime is not so great.

## rngBetter.h API
We analyzed the 60 full period generators (found through exhaustive search on an AVR). All of them were optimized (since they all do the same operations, it was posssible to autogenerate these for the most part). The full 65535 possible resultswere generated on an AVR, with tallies of key values retained - repeated nybbles, repeated bytes, repeated ends, repeated middle, and two nybbles having the same value consecutively. This is the worst behavior for my application of this. It was also tested whether there was an even distribution in the difference in value from two consecutive calls. On the worst generators, there was a VERY strong correlation here. This is bad for most other uses of a prng.

We picked the fastest one that had no obvious defects in the form of it's output (3d9), and made that the basic xor16. You can use any of the generators however, see the appendices


### Basics
The basic API amounts to just two functions:

`uint16_t xor16()` - Assuming the generator has been seeded, will generate a 16 bit pseudorandom number. If the generator has not been seeded, it will return 0.

`bool seed_xor16(uint16_t seed)` - Returns true unless the seed passed in was 0, in which case the call has no effect and returns false.

Three, if you count the one that combines both of the above.

`uint16_t xor16(uint16_t seed)` - combines the above two.

See Appendix I for the full list of full period generators provided, these do not include the set-seed-and-get-value option.

All generators keep an independent state.

All generators must be seeded with a non-zero value. If this contains no entropy, your random numbers will be the same every time.


### RNG Utility functions
These are designed t0 assist making better seeds from numbers with low entropy.

rngutil.h contains a namespace that provides a number of additional utility functions, rng16.

In the future if generators with larger state are added, we will add additional rng namespaces to this file. carring the same functions and interfaces, just resulting in more bits of seed to use.

Under no cases shall a value larger than the number of bits in the RNG state be passed to these functions.

**Warning:** Only values of 8, 10, 12, 13, 14, 15, and 16 are accepted

* `uint16_t rng16::ADCtoSeed(uint16_t val, uint8_t valsize)`

Pass this with an ADC reading from a floating pin, especially one that often picks up noise. It is not inconceivable that it might be helpful, for generating more noise, to put a trace to nowhere on an unused pin. Maybe even run it past the DC-DC converter to pick up even more noise....

We can't make more entropy than we put in. So you probably want to perform a large accumulation,use the raw value (you're better off taking the low bits vs the high bits, because there is more likely to be a pattern in the high bits, or a preferred value, rather). Study the behavior of the ADC on your specific part when reading a floating pin, it varies widely between generations of AVR). We can only make as much entropy as you give us. If you're measuring a floating pin and find that the reported voltage that never exceeds half of full scale on a tinyAVR 1-series (0-1023, so each reading is under 512), on the maximum 64x accumulate setting, the most you could hope for is half of the possible starting points, hence half of the possible sequencces. In reality the reading is likely more constrained and hence the result worse.

On the other hand, a tiny2 or EA that was not in a hurry could take 1024 accumulated values - that would get them a 22-bit value - and take the 16 low bits, (note that this happens automatically when passed to this function). Valsize is the size of the value passed in bits (ie, it contains values between 0 and 2^valsize).
If, during a test run, it was found that this didn't generate values as high as you hoped, because the ADC pin seems to get pulled in the direction of ground, maybe you would get some milage out of amplifying it with the PGA!

* `uint16_t rng16::timeADCtoSeed(uint16_t val, uint8_t valsize, uint16_t timerval)`
This static method takes a value, which should be from a source of 8 or more bits of , and mixes it up to get a value that's got the 1's and 0's smeared out more. There are two versions, with and without passing a third argument. It is intended that you would use the time on millis or micros. And since seeding is typically done early (and at a deterministic time), this won't vary much. We truncate it to 16 low bits in the argument type, since again, it's likely being called early, when even micros [16:31] is likely almost all 0's - but if your startup includes any sort of waiting on external things, that time variation will contribute a little bit of entropy too.

Each length of the ADC value is treated slightly differently as part of the no-bit-left-behind policy.

You are never guaranteed to get a valid value from these; you should always call them in a while loop as the chance of such an occurrence is quite low, so the loop will rarely run more than once. eg:

```c

while(!seed_xor16(rng16::ADCtoSeed(analogReadEnh(MY_FLOATING_PIN, ADC_ACC64), 16))); // entropy <= (max-min)*(2^16)


while(!seed_xor16(rng16::ADCtoSeed(analogReadEnh(MY_FLOATING_PIN, ADC_ACC1024), 16))); //entropy <= 2^16 as long as you can get it to pick up just 1/64th Vdd of noise a a floating pin.

while(!seed_xor16(rng16::timeADCtoSeed(analogReadEnh(MY_FLOATING_PIN, ADC_ACC64), 16, micros()));
// If your code does not wait on anything external this is no better than 1. But if it does it adds some entropy.
// note that micros() gets truncated to the low 16 bits.
```

Normally the raw accumulation values aren't good because the lower bits are just noise. But that's what we want here!

The idea is to use these to generate better values for the seeds you initialize the rng with, instead of just doing `seed_xor16(analogRead(MY_FLOATING_PIN))`

### Details
xorshift was discovered by George Marsaglia circa 2003 - surprisingly recently; his implementation concerned 32-bit generators which maintained 128b of state. This can be done on an Arduino, yes - but you damned sure don't want to if you need performance, because you need so much scratch space that it doesn't all fit into working registers, and each actual shift performed would take 8 times as long, and the tricks we used in the asm don't work either

We don't need *great* random numbers though, we just need ones that aren't total garbage and don't take forever to generate.

There are 60 distinct random number generators with a full 2<sup>16</sup>-1 period of this form with 16 bits of state.
```c
y ^= y << a;
y ^= y >> b;
y ^= y << c;
```
For each triplet of numbers (a, b, c) there are four generators, of which two are of a different character than the other two.

Swapping the position of a and c is not equivalent, and produces generators of differing qualities, however they are related in at least one way - generator abc will be full period if and only if generator cba is as well, though the two generators won't have the same clumpiness of their output - it appears that changing the order of the first and last swap does change the generator in a non-trivial way. Obviously, the two have the same

However, swapping all three swap directions ( >> a, << b, >> c), preserves symmetry: if you reverse the bits in the seed, and the direction of the swaps, the outputs will be the same as the initial function, only with the bit order reversed; those are not considered to be distinct; they are mirrors of other generators. We will call the two configurations LRL and RLR for obvious reasons. Seeded with 0x0100, LRLabc will generate values with the bits in opposite order as RLRabc seeded with 0x0080, thus making them equivalent to eachother.



Not all of them are equally unpredictable nor equally performant, and many, particularly when all the shifts are either large or small, are vulnerable to having obviously correlated values on consecutive calls. The described use case is hurt particularly badly when there are long runs, many times what would be expected by chance, where a single nybble gets "stuck" on some specific value.

Testing was conducted by calculating the 65535 values in order, and checking consecutive values for "stuck" nybbles, and pairs of 2 nybbles in the word that were the same as eachother (ex 4524, b97b has the first and last nybble matching in both), and they were scored on the maximum number of those recorded in a row. They were also compared by checking the difference between consecutive values, and sorting by bin (16 bins were used) based on magnitude - we would expect an unbiased distribution here - some were systematically biased towards small positive differences, and/or had some ranges of difference that were more common and others less common, with 8 buckets high and 8 low in count. The largest positive and negative difference from the expected totals resulted in a "terrible" rating if either of them were much over 1000, while the highest rating required they be below 50. Based on these qualities, the data sorted itself into four classes of generator quality.

There were 16 flagrantly horrible xorshift possibilities, all but 2 of which were terrible regardless of the order of the leftshifts, and those were still pretty bad backwards. There were 13 more that weren't much better (2 of those were the terribles in the opposite order, while one rated a category higher (good) in the opposite order. These were ones displaying either strong biases in the difference between consecutive values or frequent "stuck" digits. There were found to be 15 very good ones, 10 of which were very good regardless of which order the leftshifts were done in, and 3 more where they were okay in the other order. None were very good in one order and bad or terrible in the other. There were 10 that were decent, and of those 4 were part of a pair that was good in both directions, and 2 were okay in the other direction, 3 very good in the other direction and only 1 bad in the other direction.

The results of the difference and repeat tests were very strongly correlated. This builds confidence in the measures. The low nybble was by far the most likely to have long repeating stretches, and the worst had runs up to 12 values long, generally with 8 consecutive repeated low bytes (7, 9, and in one case 12). There were no generators with the longest repeated nybble sequence between 6 and 12, nor any with 12-repeat nybbles without 7 or more repeated bytes. These anomalies are far more prominent on the low versus the high side (remember high means a small negative change here), which makes sense with more left shifts than right shifts.

You can see that while there are differences between the two orders that the shifts can be performed in, sometimes significant ones, there is a very strong trend for the c-b-a and a-b-c orders to sink or swim together. There isn't much correlation between execution time and quality, however, beyond that the 2 fastest were terrible. A terrible one was tied with a very good one for third fastest, the slowest one was middling, and the two second slowest was a tie between a bad and very good one.

* 3-13-9 is an excellent combination - 29 clocks, both very good.
* 5-9-8 is an excellent combination - 30 clocks, both very good.
* 4-11-11 is an excellent combination - 33 clocks, both very good.
* 4-3-7 is supposedly good, but a bit slow.
* A 2 anywhere, or b = 15, b = 1, or c = 15 are a kiss of death.
* a = 1 does not appear to be bad - but this test may be unreasonably kind to them.
* Sixes are really bad. Tens are so bad that there aren't even any full length cycles containing a shift of 10.
* The middle shift unusurprisingly has a great impact on the result.

### These are implemented in assembly, which:
* For z = 3, 4, 5, or 6 (where z is a, b or c), assembly costs 2z-5 instruction words (1, 3, 5, 7) vs. compiler output (in these cases, the fastest path is , but reduces execution time from 5n to 2n (15, 20, 30, or 35 to 6, 8, 10, or 12 (!!!). A shift of 6 is the most costly either way. The compiler behavior here is unarguably correct and what we ask for when we say `-Os` but in a rng that is called often, we want to make the opposite tradeoff.
* For z > 6, This saves both time and flash. Unlike the above case, where the asm is ever so slightly larger, here it is both smaller and faster; the compiler generated output is pathological - a symptom of dysfunction in the optimizer. Things like clearing a register before copying another value over it, which could never be useful.
* In the cases of n = 15 and n = 14, slight improvements in size and speed were achieved by clearing a scratch register with eor, then using bst and bld to get the bit from y to the scratch register before eoring it with y, thus taking 4 for 15 and 6 for 14. The compiler doesn't seem to know about bst and bld (and to be fair, at 2 instructions per bit for a one-way ticket, they're not very appealing. ). One would love a bstc/bldc that operated from the carry bit instead (because you could combine it with rol/ror)
* For z < 3 the assembly implementation is identical to what the compiler generates.

## Appendix I - The complete xorshift RNG compendium
This fully implements ALL POSSIBLE xorshift 16-bit seed 16-bit output possibilities with full cycle length (there are 60).

The numbers after the _ indicate the shift amounts in hexadecimal

### the RNGs
```c
/* These looked good in my tests */
uint16_t xor16_174();
uint16_t xor16_17b();
uint16_t xor16_35b();
uint16_t xor16_3d9();
uint16_t xor16_437();
uint16_t xor16_4bb();
uint16_t xor16_598();
uint16_t xor16_734();
uint16_t xor16_895();
uint16_t xor16_9d3();
uint16_t xor16_b71();
uint16_t xor16_bb4();
uint16_t xor16_d3b();
/* the ones below failed, or were mediocre in my tests */
/* See appendices II and III for specifics. */
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
uint16_t xor16_3b1();
uint16_t xor16_3bb();
uint16_t xor16_471();
uint16_t xor16_57e();
uint16_t xor16_5b6();
uint16_t xor16_5bb();
uint16_t xor16_67d();
uint16_t xor16_6b5();
uint16_t xor16_6f1();
uint16_t xor16_71b();
uint16_t xor16_798();
uint16_t xor16_79d();
uint16_t xor16_7f1();
uint16_t xor16_897();
uint16_t xor16_97d();
uint16_t xor16_b17();
uint16_t xor16_b3d();
uint16_t xor16_b53();
uint16_t xor16_bb3();
uint16_t xor16_bb5();
uint16_t xor16_c13();
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
```

### The RNG-seeding functions
```c
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
```
## Appendix II: All full period 16-bit XORSHIFT generators, by shifts, ascending.
Clock is the number of clocks per call including typical overhead - of this, all but  12 are used for the math thus the math runs from 13-33 clocks.



|  a | b  | c  | clk| Function    | Rating   | Notes
|----|----|----|----|-------------|----------|---------------
|  1 |  1 | 14 | 26 | xor16_11e() | Terrible |
|  1 |  1 | 15 | 28 | xor16_11f() | Terrible |
|  1 |  5 |  2 | 37 | xor16_152() | V. poor  |
|  1 |  7 |  4 | 36 | xor16_174() | V. good  | Recommended - though I do worry about how small the shifts are
|  1 |  7 | 11 | 30 | xor16_17b() | V. good  | Recommended - though I do worry about how small the shifts are
|  1 | 11 |  3 | 31 | xor16_1b3() | Okay     |
|  1 | 15 |  6 | 36 | xor16_1f6() | Terrible |
|  1 | 15 |  7 | 29 | xor16_1f7() | Terrible |
|  2 |  5 |  1 | 37 | xor16_251() | Terrible |
|  2 |  5 | 13 | 37 | xor16_25d() | V. poor  |
|  2 |  5 | 15 | 36 | xor16_25f() | Terrible |
|  2 |  7 | 13 | 32 | xor16_27d() | V. poor  |
|  2 |  7 | 15 | 31 | xor16_27f() | Terrible |
|  3 |  1 | 12 | 30 | xor16_31c() | Terrible |
|  3 |  1 | 15 | 30 | xor16_31f() | Terrible |
|  3 |  5 | 11 | 39 | xor16_35b() | V. good  | Recommended
|  3 | 11 |  1 | 31 | xor16_3b1() | Okay     |
|  3 | 11 | 11 | 31 | xor16_3bb() | V. good  |
|  3 | 13 |  9 | 29 | xor16_3d9() | V. good  | default xor16, fastest top rated.
|  4 |  3 |  7 | 40 | xor16_437() | V. good  | Recommended - though another one that concerns me over the small shift values
|  4 |  7 |  1 | 36 | xor16_471() | Good     |
|  4 | 11 | 11 | 33 | xor16_4bb() | V. good  | Recommended
|  5 |  7 | 14 | 39 | xor16_57e() | V. poor  |
|  5 |  9 |  8 | 30 | xor16_598() | V. good  | Recommended
|  5 | 11 |  6 | 45 | xor16_5b6() | Okay     | Slowest
|  5 | 11 | 11 | 35 | xor16_5bb() | V. poor  |
|  6 |  7 | 13 | 40 | xor16_67d() | V. poor  |
|  6 | 11 |  5 | 45 | xor16_6b5() | Okay     | Slowest
|  6 | 15 |  1 | 36 | xor16_6f1() | Terrible |
|  7 |  1 | 11 | 30 | xor16_71b() | Terrible |
|  7 |  3 |  4 | 40 | xor16_734() | V. good  | Recommended - though I do worry about how small the shifts are
|  7 |  9 |  8 | 25 | xor16_798() | Okay     | Fastest
|  7 |  9 | 13 | 28 | xor16_79c() | Okay     |
|  7 | 15 |  1 | 29 | xor16_7f1() | Terrible |
|  8 |  9 |  5 | 30 | xor16_895() | V. good  | Recommended
|  8 |  9 |  7 | 25 | xor16_897() | Okay     | Fastest
|  9 |  7 | 13 | 28 | xor16_97d() | Good     |
|  9 | 13 |  3 | 29 | xor16_9d3() | V. good  | Recommended, fastest top rated.
| 11 |  1 |  7 | 30 | xor16_b17() | Terrible |
| 11 |  3 | 13 | 31 | xor16_b3d() | Good     |
| 11 |  5 |  3 | 39 | xor16_b53() | V.Good   |
| 11 |  7 |  1 | 30 | xor16_b71() | V. good  | Recommended - though I do worry about how small the shifts are
| 11 | 11 |  3 | 31 | xor16_bb3() | Good     |
| 11 | 11 |  4 | 33 | xor16_bb4() | V. good  | Recommended
| 11 | 11 |  5 | 35 | xor16_bb5() | V. poor  |
| 12 |  1 |  3 | 30 | xor16_c13() | Terrible |
| 12 |  3 | 13 | 30 | xor16_c3d() | Good     |
| 13 |  3 | 11 | 31 | xor16_d3b() | V. good  | Recommended
| 13 |  3 | 12 | 30 | xor16_d3c() | Good     |
| 13 |  5 |  2 | 37 | xor16_d52() | V. poor  |
| 13 |  7 |  2 | 32 | xor16_d72() | V. poor  |
| 13 |  7 |  6 | 40 | xor16_d76() | V. poor  |
| 13 |  7 |  9 | 28 | xor16_d79() | Good     |
| 13 |  9 |  7 | 28 | xor16_c97() | Okay     |
| 14 |  1 |  1 | 26 | xor16_e11() | Terrible |
| 14 |  7 |  5 | 39 | xor16_e75() | Good     |
| 15 |  1 |  1 | 28 | xor16_f11() | Terrible |
| 15 |  1 |  3 | 30 | xor16_f13() | Terrible |
| 15 |  5 |  2 | 36 | xor16_f52() | V. poor  |
| 15 |  7 |  2 | 31 | xor16_f72() | V. poor  |

## Appendix III: All full period 16-bit XORSHIFT generators, by rating (dec)
|  a |  b |  c | Clk | Function    | Rating
|----|----|----|-----|-------------|----------
|  9 | 13 |  3 |  29 | xor16_9d3() | V. good
|  3 | 13 |  9 |  29 | xor16_3d9() | V. good
|  1 |  7 | 11 |  30 | xor16_17b() | V. good
|  5 |  9 |  8 |  30 | xor16_598() | V. good
| 11 |  7 |  1 |  30 | xor16_b71() | V. good
|  8 |  9 |  5 |  30 | xor16_895() | V. good
|  3 | 11 | 11 |  31 | xor16_3bb() | V. good
| 13 |  3 | 11 |  31 | xor16_d3b() | V. good
|  4 | 11 | 11 |  33 | xor16_4bb() | V. good
| 11 | 11 |  4 |  33 | xor16_bb4() | V. good
|  1 |  7 |  4 |  36 | xor16_174() | V. good, but slow.
| 11 |  5 |  3 |  39 | xor16_b53() | V. good, but slow.
|  3 |  5 | 11 |  39 | xor16_35b() | V. good, but slow.
|  4 |  3 |  7 |  40 | xor16_437() | V. good, but slow.
|  7 |  3 |  4 |  40 | xor16_734() | V. good, but slow.
|  9 |  7 | 13 |  28 | xor16_97d() | Good
| 13 |  7 |  9 |  28 | xor16_d79() | Good
| 12 |  3 | 13 |  30 | xor16_c3d() | Good
| 13 |  3 | 12 |  30 | xor16_d3c() | Good
| 11 |  3 | 13 |  31 | xor16_b3d() | Good
| 11 | 11 |  3 |  31 | xor16_bb3() | Good
|  4 |  7 |  1 |  36 | xor16_471() | Good, but slow.
| 14 |  7 |  5 |  39 | xor16_e75() | Good, but slow.
|  7 |  9 |  8 |  25 | xor16_798() | Okay - the fastest, though for only 4 more clocks you can get good random numbers instead.
|  8 |  9 |  7 |  25 | xor16_897() | Okay - the fastest, though for only 4 more clocks you can get good random numbers instead.
|  7 |  9 | 13 |  28 | xor16_79c() | Okay
| 13 |  9 |  7 |  28 | xor16_c97() | Okay
|  1 | 11 |  3 |  31 | xor16_1b3() | Okay
|  3 | 11 |  1 |  31 | xor16_3b1() | Okay
|  5 | 11 |  6 |  45 | xor16_5b6() | Okay - but very slow!
|  6 | 11 |  5 |  45 | xor16_6b5() | Okay - but very slow!
| 15 |  7 |  2 |  31 | xor16_f72() | V. poor
|  2 |  7 | 13 |  32 | xor16_27d() | V. poor
| 13 |  7 |  2 |  32 | xor16_d72() | V. poor
|  5 | 11 | 11 |  35 | xor16_5bb() | V. poor
| 11 | 11 |  5 |  35 | xor16_bb5() | V. poor
| 15 |  5 |  2 |  36 | xor16_f52() | V. poor - and slow!
|  1 |  5 |  2 |  37 | xor16_152() | V. poor - and slow!
|  2 |  5 | 13 |  37 | xor16_25d() | V. poor - and slow!
| 13 |  5 |  2 |  37 | xor16_d52() | V. poor - and slow!
|  5 |  7 | 14 |  39 | xor16_57e() | V. poor - and slow!
|  6 |  7 | 13 |  40 | xor16_67d() | V. poor - and slow!
| 13 |  7 |  6 |  40 | xor16_d76() | V. poor - and slow!
|  1 |  1 | 14 |  26 | xor16_11e() | Terrible
| 14 |  1 |  1 |  26 | xor16_e11() | Terrible
|  1 |  1 | 15 |  28 | xor16_11f() | Terrible
| 15 |  1 |  1 |  28 | xor16_f11() | Terrible
|  1 | 15 |  7 |  29 | xor16_1f7() | Terrible
|  7 | 15 |  1 |  29 | xor16_7f1() | Terrible
|  3 |  1 | 12 |  30 | xor16_31c() | Terrible
|  3 |  1 | 15 |  30 | xor16_31f() | Terrible
| 15 |  1 |  3 |  30 | xor16_f13() | Terrible
| 11 |  1 |  7 |  30 | xor16_b17() | Terrible
| 12 |  1 |  3 |  30 | xor16_c13() | Terrible
|  7 |  1 | 11 |  30 | xor16_71b() | Terrible
|  2 |  7 | 15 |  31 | xor16_27f() | Terrible
|  6 | 15 |  1 |  36 | xor16_6f1() | Terrible - and slow!
|  2 |  5 | 15 |  36 | xor16_25f() | Terrible - and slow!
|  1 | 15 |  6 |  36 | xor16_1f6() | Terrible - and slow!
|  2 |  5 |  1 |  37 | xor16_251() | Terrible - and slow!

## Appendix IV: All full period 16-bit XORSHIFT generators, by speed (dec)
|  a |  b |  c | Clk | Function    | Rating
|----|----|----|-----|-------------|----------
|  7 |  9 |  8 |  25 | xor16_798() | Okay - the fastest, though for only 4 more clocks you can get good random numbers instead.
|  8 |  9 |  7 |  25 | xor16_897() | Okay - the fastest, though for only 4 more clocks you can get good random numbers instead.
|  1 |  1 | 14 |  26 | xor16_11e() | Terrible
| 14 |  1 |  1 |  26 | xor16_e11() | Terrible
|  9 |  7 | 13 |  28 | xor16_97d() | Good
| 13 |  7 |  9 |  28 | xor16_d79() | Good
|  7 |  9 | 13 |  28 | xor16_79c() | Okay
| 13 |  9 |  7 |  28 | xor16_c97() | Okay
|  1 |  1 | 15 |  28 | xor16_11f() | Terrible
| 15 |  1 |  1 |  28 | xor16_f11() | Terrible
|  9 | 13 |  3 |  29 | xor16_9d3() | V. good
|  3 | 13 |  9 |  29 | xor16_3d9() | V. good
|  1 | 15 |  7 |  29 | xor16_1f7() | Terrible
|  7 | 15 |  1 |  29 | xor16_7f1() | Terrible
|  1 |  7 | 11 |  30 | xor16_17b() | V. good
|  5 |  9 |  8 |  30 | xor16_598() | V. good
| 11 |  7 |  1 |  30 | xor16_b71() | V. good
|  8 |  9 |  5 |  30 | xor16_895() | V. good
| 12 |  3 | 13 |  30 | xor16_c3d() | Good
| 13 |  3 | 12 |  30 | xor16_d3c() | Good
|  3 |  1 | 12 |  30 | xor16_31c() | Terrible
|  3 |  1 | 15 |  30 | xor16_31f() | Terrible
| 15 |  1 |  3 |  30 | xor16_f13() | Terrible
| 11 |  1 |  7 |  30 | xor16_b17() | Terrible
| 12 |  1 |  3 |  30 | xor16_c13() | Terrible
|  7 |  1 | 11 |  30 | xor16_71b() | Terrible
|  3 | 11 | 11 |  31 | xor16_3bb() | V. good
| 13 |  3 | 11 |  31 | xor16_d3b() | V. good
| 11 |  3 | 13 |  31 | xor16_b3d() | Good
| 11 | 11 |  3 |  31 | xor16_bb3() | Good
|  1 | 11 |  3 |  31 | xor16_1b3() | Okay
|  3 | 11 |  1 |  31 | xor16_3b1() | Okay
| 15 |  7 |  2 |  31 | xor16_f72() | V. poor
|  2 |  7 | 15 |  31 | xor16_27f() | Terrible
|  2 |  7 | 13 |  32 | xor16_27d() | V. poor
| 13 |  7 |  2 |  32 | xor16_d72() | V. poor
|  4 | 11 | 11 |  33 | xor16_4bb() | V. good
| 11 | 11 |  4 |  33 | xor16_bb4() | V. good
|  5 | 11 | 11 |  35 | xor16_5bb() | V. poor
| 11 | 11 |  5 |  35 | xor16_bb5() | V. poor
|  1 |  7 |  4 |  36 | xor16_174() | V. good, but slow.
|  4 |  7 |  1 |  36 | xor16_471() | Good, but slow.
|  6 | 15 |  1 |  36 | xor16_6f1() | Terrible - and slow!
|  2 |  5 | 15 |  36 | xor16_25f() | Terrible - and slow!
|  1 | 15 |  6 |  36 | xor16_1f6() | Terrible - and slow!
| 15 |  5 |  2 |  36 | xor16_f52() | V. poor - and slow!
|  1 |  5 |  2 |  37 | xor16_152() | V. poor - and slow!
|  2 |  5 | 13 |  37 | xor16_25d() | V. poor - and slow!
| 13 |  5 |  2 |  37 | xor16_d52() | V. poor - and slow!
|  2 |  5 |  1 |  37 | xor16_251() | Terrible - and slow!
| 11 |  5 |  3 |  39 | xor16_b53() | V. good, but slow.
|  3 |  5 | 11 |  39 | xor16_35b() | V. good, but slow.
| 14 |  7 |  5 |  39 | xor16_e75() | Good, but slow.
|  5 |  7 | 14 |  39 | xor16_57e() | V. poor - and slow!
|  4 |  3 |  7 |  40 | xor16_437() | V. good, but slow.
|  7 |  3 |  4 |  40 | xor16_734() | V. good, but slow.
|  6 |  7 | 13 |  40 | xor16_67d() | V. poor - and slow!
| 13 |  7 |  6 |  40 | xor16_d76() | V. poor - and slow!
|  5 | 11 |  6 |  45 | xor16_5b6() | Okay - but very slow!
|  6 | 11 |  5 |  45 | xor16_6b5() | Okay - but very slow!

### Appendix V: Limitations of this investigation
* Only 16-bit RNGs have been considered at this time, as that was what was needed for the application.
* The statistical tests are incredibly crude. they could be done much more rigorously.
  * Guided by the application needs this was focused on generating random binary digits with plans to bitslice them up.
* One can imagine an equivalent with rol and ror instead of lsl and lsr. It is not known what impact changing this would have.
  * Note that the difference I refer to is what value should get shifted in, a 1 or a 0. >> and << always shift in 0's. but one could imagine doing what other architectures do and rotating the bits without a carry register holding a bit. So to make an rol or ror for multibyte values, we'd have to define what exactly it meant. Is x rol32 (y = x << y) | (x >> (32-y)) (no carry bit - and also the slowest way to calculate it possible) or is it? (x << y) | (x >> (33-y)) (notice that this introduces a gap)
  * Other approaches of PRNG based on different instructions and/or for different architectures may be superior to the methods employed herein within appropriate contexts.
