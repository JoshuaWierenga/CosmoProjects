#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TYPE BINARY
#define SIZE 80

#ifndef TYPE
#error Floating point type not specified

#elif !defined(SIZE)
#error Floating point size not specified

//TODO Ensure that FLT_RADIX and other macro values match expected values
#elif TYPE == BINARY && SIZE == 32
#define FLT_T float
#define FLT_MAX_VAL FLT_MAX
// 0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625 == 2^-126 is 128 chars long
#define FLT_MAX_LEN 128
//   000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625 is 126 chars long
#define PRI_FLT ".126f"
#define FLT_POW powf

#define FLT_BASE 2

#define FLT_EXP_T int_fast8_t
#define PRI_FLT_EXP PRIdFAST8
#define FLT_EXP_MIN_VALUE -126
#define FLT_EXP_MAX_VALUE 127
#define FLT_EXP_DIG_COUNT 3

#elif TYPE == BINARY && SIZE == 64
#define FLT_T double
#define FLT_MAX_VAL DBL_MAX
// GUESS: 2^11 or FLT_EXP_MAX_VALUE + 1
#define FLT_MAX_LEN 1024
// GUESS: 2^11 - 2 or FLT_EXP_MAX_VALUE - 1 or -FLT_EXP_MIN_VALUE
#define PRI_FLT ".1022f"
#define FLT_POW pow

#define FLT_BASE 2

#define FLT_EXP_T int_fast16_t
#define PRI_FLT_EXP PRIdFAST16
#define FLT_EXP_MIN_VALUE -1022
#define FLT_EXP_MAX_VALUE 1023
#define FLT_EXP_DIG_COUNT 4

#elif TYPE == BINARY && SIZE == 80
#define FLT_T long double
#define FLT_MAX_VAL LDBL_MAX
// GUESS: 2^14
#define FLT_MAX_LEN 16384
// GUESS: 2^14 - 2
#define PRI_FLT ".16382Lf"
#define FLT_POW powl

#define FLT_BASE 2

#define FLT_EXP_T int_fast16_t
#define PRI_FLT_EXP PRIdFAST16
// GUESS: -PRI_FLT
#define FLT_EXP_MIN_VALUE -16382
// GUESS: FLT_MAX_LEN - 1
#define FLT_EXP_MAX_VALUE 16383
#define FLT_EXP_DIG_COUNT 5

#else
#error Invalid combination of floating point type and size provided
#endif

#define DEBUG false

void printFloat(FLT_T f) {
  int length = snprintf(NULL, 0, "%" PRI_FLT, f);
  
  char strFloat[length + 1];
  snprintf(strFloat, sizeof strFloat, "%" PRI_FLT, f);
  for (int i = length - 1; i >= 0 && strFloat[i] == '0'; i--) {
      strFloat[i] = '\0';
  }
  
  printf("%s", strFloat);
  if (length != strlen(strFloat), FLT_MAX_LEN - strlen(strFloat)) {
    printf("%0*d", FLT_MAX_LEN - strlen(strFloat), 0);
  }
  
  if (DEBUG) {
    printf(", %*i, %*i, %*i", FLT_EXP_DIG_COUNT, FLT_EXP_DIG_COUNT, FLT_EXP_DIG_COUNT, length, strlen(strFloat), FLT_MAX_LEN - strlen(strFloat));
  }
}

// Using int_fast16_t for p1 so that displayMaxValue works for float where 128 is not in int_fast8_t's [-128, 127]
void printPowers(uint_fast8_t b1, int_fast16_t p1, uint_fast8_t b2, FLT_EXP_T p2, char sign) {
  int length = printf("%" PRIuFAST8 "^%" PRIdFAST16, b1, p1);
  
  // TODO: Figure out why its 10 exactly
  if (b2 == 0) {
    printf("%*s= ", 10 + 2 * FLT_EXP_DIG_COUNT - length, "");
  } else {
    int length2 = snprintf(NULL, 0, " + %" PRIuFAST8 "^%" PRI_FLT_EXP, b2, p2);
    printf(" %c %" PRIuFAST8 "^%" PRI_FLT_EXP "%*s= ", sign, b2, p2, 10 + 2 * FLT_EXP_DIG_COUNT - length - length2, ""); 
  }
}

FLT_EXP_T findSmallestIncrease(FLT_T value) {
  if (DEBUG) {
    puts("Finding smallest larger value");
  }
  
  FLT_EXP_T smallestPower;
  for (smallestPower = FLT_EXP_MAX_VALUE; smallestPower >= FLT_EXP_MIN_VALUE; --smallestPower) {
    FLT_T newValue = value + FLT_POW(FLT_BASE, smallestPower);
    
    // Yes, exact float comparision
    if (newValue == value) {
        break;
    }
    
    if (DEBUG) {
      printf("%4" PRI_FLT_EXP ":   ", smallestPower);
      printFloat(newValue);
      putchar('\n');
    }
  }
  
  return smallestPower + 1;
}

void displaySmallestIncreases() {
  FLT_T value = 0;
  for (FLT_EXP_T currentPower = FLT_EXP_MIN_VALUE; currentPower <= FLT_EXP_MAX_VALUE && value < FLT_POW(FLT_BASE, FLT_EXP_MAX_VALUE); ++currentPower) {
    value = FLT_POW(FLT_BASE, currentPower);
    
    printf("Value: ");
    printPowers(FLT_BASE, currentPower, 0, 0, '+');
    printFloat(value);
    putchar('\n');
    
    FLT_EXP_T smallestPower = findSmallestIncrease(value);
    
    if (DEBUG) {
      printf("Smallest possible increase is: \n        ");
      printFloat(FLT_POW(FLT_BASE, smallestPower));
    } else {
      printf("Next:  ");
      printPowers(FLT_BASE, currentPower, FLT_BASE, smallestPower, '+');
      printFloat(value + FLT_POW(FLT_BASE, smallestPower));
    }
    puts("\n");
  }
}

void displayMaxValue() {
  FLT_T maxPower = FLT_POW(FLT_BASE, FLT_EXP_MAX_VALUE);
  FLT_EXP_T smallestExp = findSmallestIncrease(maxPower);
  
  printf("\nMax:   ");
  printPowers(FLT_BASE, FLT_MAX_LEN, FLT_BASE, smallestExp, '-');
  printFloat(FLT_MAX_VAL);
  putchar('\n');
}

int main() {
  // TODO Mention subnormals
  
  displaySmallestIncreases();
  displayMaxValue();
}
