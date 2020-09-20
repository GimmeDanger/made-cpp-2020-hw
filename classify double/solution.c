#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Numerical constants for special bits in IEEE754 double model.
 * All of them are provided in hex format,
 * Unit tests are included for correctness
*/

#define MANTISSA_RANGE_BEGIN (0)
#define MANTISSA_RANGE_END   (52)
#define EXPONENT_RANGE_BEGIN (MANTISSA_RANGE_END)
#define EXPONENT_RANGE_END   (63)
#define SIGN_BIT_POS         (EXPONENT_RANGE_END)
#define QNAN_BIT_POS         (MANTISSA_RANGE_END-1)

/*  signed bit              <--- mantissa --->
 *  |             |                                                  |
 *  1 00000000000 0000000000000000000000000000000000000000000000000000
 *    |         |
 *     exponent
 */
#define ONLY_SIGNED_BIT 0x8000000000000000

/*  signed bit              <--- mantissa --->
 *  |             |                                                  |
 *  0 00000000000 1000000000000000000000000000000000000000000000000000
 *    |         |
 *     exponent
 */
#define ONLY_QNAN_BIT 0x0008000000000000

/*  signed bit              <--- mantissa --->
 *  |             |                                                  |
 *  0 11111111111 0000000000000000000000000000000000000000000000000000
 *    |         |
 *     exponent
 */
#define ONLY_EXPONENT_BITS 0x7FF0000000000000

/*  signed bit              <--- mantissa --->
 *  |             |                                                  |
 *    00000000000 1111111111111111111111111111111111111111111111111111
 *    |         |
 *     exponent
 */
#define ONLY_MANTISSA_BITS 0x000FFFFFFFFFFFFF

/**
 * Library-level functions.
 * You should use them in the main sections.
 */

uint64_t convertToUint64(double number) {
  return *((uint64_t *)(&number));
}

bool isSignedBitSet(uint64_t number) {
  return number & ONLY_SIGNED_BIT;
}

bool isQNANBitSet(uint64_t number) {
  return number & ONLY_QNAN_BIT;
}

bool allExponentBitsSet(uint64_t number) {
  return (number & ONLY_EXPONENT_BITS) == ONLY_EXPONENT_BITS;
}

bool noneExponentBitsSet(uint64_t number) {
  return (number & ONLY_EXPONENT_BITS) == 0;
}

bool anyExponentBitsSet(uint64_t number) {
  return (number & ONLY_EXPONENT_BITS);
}

bool allMantissaBitsSet(uint64_t number) {
  return (number & ONLY_MANTISSA_BITS) == ONLY_MANTISSA_BITS;
}

bool noneMantissaBitsSet(uint64_t number) {
  return (number & ONLY_MANTISSA_BITS) == 0;
}

bool anyMantissaBitsSet(uint64_t number) {
  return (number & ONLY_MANTISSA_BITS);
}

/**
 * Checkers here:
*/

bool checkForNegative(uint64_t number) {
  return isSignedBitSet(number);
}

bool checkForPositive(uint64_t number) {
  return !isSignedBitSet(number);
}

//       sgn     exp         man
// +0 = { 0 | 00 ... 00 | 00 ... 00}
// -0 = { 1 | 00 ... 00 | 00 ... 00}
bool checkForZero(uint64_t number) {
  return noneExponentBitsSet(number)
      && noneMantissaBitsSet(number);
}

bool checkForPlusZero(uint64_t number) {
  return checkForPositive(number)
      && checkForZero(number);
}

bool checkForMinusZero(uint64_t number) {
  return checkForNegative(number)
      && checkForZero(number);
}

//         sgn     exp         man
// +inf = { 0 | 11 ... 11 | 00 ... 00}
// -inf = { 1 | 11 ... 11 | 00 ... 00}

bool checkForInf(uint64_t number) {
  return allExponentBitsSet(number)
      && noneMantissaBitsSet(number);
}

bool checkForPlusInf(uint64_t number) {
  return checkForPositive(number)
      && checkForInf(number);
}

bool checkForMinusInf(uint64_t number) {
  return checkForNegative(number)
      && checkForInf(number);
}

//                sgn     exp         man
//  normalized = { X | 00 ... 01 | XX ... XX}
//                        ...
//               { X | 11 ... 10 | XX ... XX}
bool checkForNormal(uint64_t number) {
  return !allExponentBitsSet(number)
      && !noneExponentBitsSet(number);
}

bool checkForPlusNormal(uint64_t number) {
  return checkForPositive(number)
      && checkForNormal(number);
}

bool checkForMinusNormal(uint64_t number) {
  return checkForNegative(number)
      && checkForNormal(number);
}

//                  sgn     exp         man
//  denormalized = { X | 00 ... 00 | 00 ... 01}
//                          ...
//                 { X | 00 ... 00 | 11 ... 11}
bool checkForDenormal(uint64_t number) {
  return noneExponentBitsSet(number)
      && anyMantissaBitsSet(number);
}

bool checkForPlusDenormal(uint64_t number) {
  return checkForPositive(number)
      && checkForDenormal(number);
}

bool checkForMinusDenormal(uint64_t number) {
  return checkForNegative(number)
      && checkForDenormal(number);
}

//         sgn     exp         man
//  NaN = { X | 11 ... 11 | 00 ... 01}
//                             ...
//        { X | 11 ... 11 | 11 ... 11}
bool checkForNan(uint64_t number) {
  return allExponentBitsSet(number)
      && anyMantissaBitsSet(number);
}

//          sgn     exp         man
//  SNaN = { X | 11 ... 11 | 00 ... 01}
//                              ...
//         { X | 11 ... 11 | 01 ... 11}
bool checkForSignalingNan (uint64_t number) {
  return checkForNan(number)
      && !isQNANBitSet(number);
}

//           sgn     exp         man
//  SNaN = { X | 11 ... 11 | 1X ... XX}
bool checkForQuietNan (uint64_t number) {
  return checkForNan(number)
      && isQNANBitSet(number);
}

void classify(double number) {
  uint64_t unumber = convertToUint64(number);
  if (checkForPlusZero(unumber)) {
    printf("Plus zero\n");
  }
  else if (checkForMinusZero(unumber)) {
    printf("Minus zero\n");
  }
  else if (checkForPlusInf(unumber)) {
    printf("Plus inf\n");
  }
  else if (checkForMinusInf(unumber)) {
    printf("Minus inf\n");
  }
  else if (checkForPlusNormal(unumber)) {
    printf("Plus normal\n");
  }
  else if (checkForMinusNormal(unumber)) {
    printf("Minus normal\n");
  }
  else if (checkForPlusDenormal(unumber)) {
    printf("Plus Denormal\n");
  }
  else if (checkForMinusDenormal(unumber)) {
    printf("Minus Denormal\n");
  }
  else if (checkForSignalingNan(unumber)) {
    printf("Signailing NaN\n");
  }
  else if (checkForQuietNan(unumber)) {
    printf("Quiet NaN\n");
  }
  else {
    printf("Error.\n");
  }
}

/**
 * Unit tests here:
*/

bool getBit(uint64_t number, uint8_t index) {
  return (number >> index) & 1;
}

bool checkAllBitRange(uint64_t number, uint8_t begin, uint8_t end) {
  for (uint8_t pos = begin; pos < end; pos++)
    if (!getBit(number, pos))
      return false;
  return true;
}

bool checkNoneBitRange(uint64_t number, uint8_t begin, uint8_t end) {
  for (uint8_t pos = begin; pos < end; pos++)
    if (getBit(number, pos))
      return false;
  return true;
}

bool checkAnyBitRange(uint64_t number, uint8_t begin, uint8_t end) {
  bool any_bit_found = false;
  for (uint8_t pos = begin; pos < end; pos++)
    any_bit_found |= getBit(number, pos);
  return any_bit_found;
}

void unitTestsForNumberStorageScheme() {
  // check ONLY_MANTISSA_BITS
  assert(checkAllBitRange(ONLY_MANTISSA_BITS, MANTISSA_RANGE_BEGIN, MANTISSA_RANGE_END));
  assert(checkNoneBitRange(ONLY_MANTISSA_BITS, EXPONENT_RANGE_BEGIN, EXPONENT_RANGE_END));
  assert(!getBit(ONLY_MANTISSA_BITS, SIGN_BIT_POS));
  // check ONLY_EXPONENT_BITS
  assert(checkNoneBitRange(ONLY_EXPONENT_BITS, MANTISSA_RANGE_BEGIN, MANTISSA_RANGE_END));
  assert(checkAllBitRange(ONLY_EXPONENT_BITS, EXPONENT_RANGE_BEGIN, EXPONENT_RANGE_END));
  assert(!getBit(ONLY_EXPONENT_BITS, SIGN_BIT_POS));
  // check ONLY_SIGNED_BIT
  assert(checkNoneBitRange(ONLY_SIGNED_BIT, MANTISSA_RANGE_BEGIN, MANTISSA_RANGE_END));
  assert(checkNoneBitRange(ONLY_SIGNED_BIT, EXPONENT_RANGE_BEGIN, EXPONENT_RANGE_END));
  assert(getBit(ONLY_SIGNED_BIT, SIGN_BIT_POS));
  // check ONLY_QNAN_BIT
  assert(checkNoneBitRange(ONLY_QNAN_BIT, MANTISSA_RANGE_BEGIN, MANTISSA_RANGE_END-1));
  assert(checkNoneBitRange(ONLY_QNAN_BIT, EXPONENT_RANGE_BEGIN, EXPONENT_RANGE_END));
  assert(getBit(ONLY_QNAN_BIT, QNAN_BIT_POS));
}

void unitTestsForZeroChecker() {
  uint64_t plus_zero = convertToUint64(0.0E0);
  uint64_t minus_zero = convertToUint64(-0.0E0);

  assert(checkForZero(plus_zero));
  assert(checkForZero(minus_zero));
  assert(checkForPlusZero(plus_zero));
  assert(checkForMinusZero(minus_zero));

  assert(!checkForMinusZero(plus_zero));
  assert(!checkForPlusZero(minus_zero));
}

void unitTestsForInfChecker() {
  uint64_t plus_inf = 0x7FF0000000000000;
  uint64_t minus_inf = 0xFFF0000000000000;

  assert(checkForInf(plus_inf));
  assert(checkForInf(minus_inf));
  assert(checkForPlusInf(plus_inf));
  assert(checkForMinusInf(minus_inf));

  assert(!checkForMinusInf(plus_inf));
  assert(!checkForPlusInf(minus_inf));
}

void unitTestsForNANChecker() {
  // all exponent bits + qnan bit + some other random bits are set
  {
    uint64_t qnan = 0x7FF8000000000000;
    assert(checkForQuietNan(qnan));
  }
  {
    uint64_t qnan = 0x7FF8010000080000;
    assert(checkForQuietNan(qnan));
  }
  {
    uint64_t qnan = 0xFFF8010000080402;
    assert(checkForQuietNan(qnan));
  }

  // all exponent bits are set, but qnan bit isn't
  {
    uint64_t not_qnan = 0x7FF0000000000000;
    assert(!checkForQuietNan(not_qnan));
  }

  // qnan bit is set, but all exponent bits aren't
  {
    uint64_t not_qnan = 0x6F78000000000000;
    assert(!checkForQuietNan(not_qnan));
  }

  // all exponent bits + qnan bit not set + some other random bits are set
  {
    uint64_t snan = 0x7FF0020400000001;
    assert(checkForSignalingNan(snan));
  }

  // all exponent bits + qnan bit set + some other random bits are set
  {
    uint64_t not_snan = 0x7FF8020400000001;
    assert(!checkForSignalingNan(not_snan));
  }

  // not all exponent bits set + qnan bit not set + some other random bits are set
  {
    uint64_t not_snan = 0x7BB4020400000001;
    assert(!checkForSignalingNan(not_snan));
  }
}

void unitTestsForRegularChecker() {
  uint64_t x = convertToUint64(-1.234567);
  assert(checkForNormal(x));
  assert(!checkForDenormal(x));

  x = convertToUint64(7.61652383986240116544603204255E287);
  assert(checkForNormal(x));
  assert(!checkForDenormal(x));

  x = convertToUint64(1.18210661101100532913354115577E-308);
  assert(!checkForNormal(x));
  assert(checkForDenormal(x));
}

int main(void) {
  // Unit tests
  unitTestsForNumberStorageScheme();
  unitTestsForZeroChecker();
  unitTestsForInfChecker();
  unitTestsForNANChecker();
  unitTestsForRegularChecker();
  printf("All tests passed!\n");

  // Classify user input
  double x;
  scanf("%lf", &x);
  classify(x);
  return 0;
}
