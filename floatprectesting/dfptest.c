#define __STDC_WANT_DEC_FP__
#define __STDC_WANT_IEC_60559_DFP_EXT__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GLIBC__
#include <quadmath.h>
#endif

// TODO: Use __float128 for binary diff checking so that test80 has somewhat working diff checking
// TODO: Use a string with manual subtraction for all diff checking?

// TODO: Generate these
// One on ...
static const char TEN[]   = "0.100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
static const char SEVEN[] = "0.142857142857142857142857142857142857142857142857142857142857142857142857142857142857142857142857142857142857142857142857";
static const char THREE[] = "0.333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333";

// TODO: Find a way to not need two _Generic uses, the functions have different parameter lists so cannot use a function pointer afaik
// TODO: Find out why manual null termination is needed for cosmo
#define printfloat(fstr, val) \
{\
  int size = _Generic((val),\
    _Float128: quadmath_snprintf,\
    _Decimal32: strfromd32,\
    _Decimal64: strfromd64,\
    _Decimal128: strfromd128\
  )(NULL, 0, fstr, val);\
  char str[size + 1];\
  _Generic((val),\
    _Float128: quadmath_snprintf,\
    _Decimal32: strfromd32,\
    _Decimal64: strfromd64,\
    _Decimal128: strfromd128\
  )(str, size, fstr, val);\
  str[size] = '\0';\
  printf("%s\n", str);\
}

void test32(int divisor, const char *truth) {
  int leftRequiredOffset = strlen("decimal32");
  float b32 = 1.0f / divisor;
  _Decimal32 d32 = 1.0df / divisor;
  
  _Decimal128 overallDiff = (_Decimal128)d32 - (_Decimal128)b32; 
  int rightRequiredOffset = (int)(overallDiff < 0);
  
  long double b32Diff;
  if (truth) {
    b32Diff = (long double)b32 - strtold(truth, NULL);
    rightRequiredOffset |= (int)(b32Diff < 0);
  }

  _Decimal128 d32Diff;
  if (truth) {
    d32Diff = (_Decimal128)d32 - strtod128(truth, NULL);
    rightRequiredOffset |= (int)(d32Diff < 0);
  }
  
  // Need 1 if no minus and 2 if minus to ensure at least one space is printed
  rightRequiredOffset++;
  
  int expLen = printf("1.0 / %i", divisor);
  printf("%*s=%*s", leftRequiredOffset - expLen + 1, "", rightRequiredOffset, "");
  if (truth) {
    printf("%.42s\n\n", truth);
  } else {
    printf("Unknown\n\n");
  }
  
  printf("binary32%*s=%*s%.40f\n", leftRequiredOffset - 7, "", rightRequiredOffset, "", b32);
  
  if (truth) {
    int rightPadding = b32Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s%.40Lf\n", leftRequiredOffset - 3, "", rightPadding, "", b32Diff);
  }
  putchar('\n');

  printf("decimal32 =%*s", rightRequiredOffset, "");
  printfloat("%.40f", d32);
  
  if (truth) {
    int rightPadding = d32Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s", leftRequiredOffset - 3, "", rightPadding, "");
    printfloat("%.40f", d32Diff);
  }
  putchar('\n');
  
  printf("d32 - b32%*s≈ ", leftRequiredOffset - 8, "");
  printfloat("%.40f", overallDiff);
  
  puts("\n");
}

void test64(int divisor, const char *truth) {
  int leftRequiredOffset = strlen("decimal64");
  double b64 = 1.0d / divisor;
  _Decimal64 d64 = 1.0dd / divisor;
  
  _Decimal128 overallDiff = (_Decimal128)d64 - (_Decimal128)b64; 
  int rightRequiredOffset = (int)(overallDiff < 0);
  
  long double b64Diff;
  if (truth) {
    b64Diff = (long double)b64 - strtold(truth, NULL);
    rightRequiredOffset |= (int)(b64Diff < 0);
  }

  _Decimal128 d64Diff;
  if (truth) {
    d64Diff = (_Decimal128)d64 - strtod128(truth, NULL);
    rightRequiredOffset |= (int)(d64Diff < 0);
  }
  
  // Need 1 if no minus and 2 if minus to ensure at least one space is printed
  rightRequiredOffset++;
  
  int expLen = printf("1.0 / %i", divisor);
  printf("%*s=%*s", leftRequiredOffset - expLen + 1, "", rightRequiredOffset, "");
  if (truth) {
    printf("%.82s\n\n", truth);
  } else {
    printf("Unknown\n\n");
  }
  
  printf("binary64%*s=%*s%.80f\n", leftRequiredOffset - 7, "", rightRequiredOffset, "", b64);
  
  if (truth) {
    int rightPadding = b64Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s%.80Lf\n", leftRequiredOffset - 3, "", rightPadding, "", b64Diff);
  }
  putchar('\n');

  printf("decimal64 =%*s", rightRequiredOffset, "");
  printfloat("%.80f", d64);
  
  if (truth) {
    int rightPadding = d64Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s", leftRequiredOffset - 3, "", rightPadding, "");
    printfloat("%.80f", d64Diff);
  }
  putchar('\n');
  
  printf("d64 - b64%*s≈ ", leftRequiredOffset - 8, "");
  printfloat("%.80f", overallDiff);
  
  puts("\n");
}

void test80(int divisor, const char *truth) {
  int leftRequiredOffset = strlen("binary80");
  long double b80 = 1.0l / divisor;
  
  // Need 1 if no minus and 2 if minus to ensure at least one space is printed
  int rightRequiredOffset = 1;
  
  long double b80Diff;
  if (truth) {
    b80Diff = b80 - strtold(truth, NULL);
    rightRequiredOffset += (int)(b80Diff < 0);
  }
  
  int expLen = printf("1.0 / %i", divisor);
  printf("%*s=%*s", leftRequiredOffset - expLen + 1, "", rightRequiredOffset, "");
  if (truth) {
    printf("%.102s\n\n", truth);
  } else {
    printf("Unknown\n\n");
  }
  
  printf("binary80%*s=%*s%.100Lf\n", leftRequiredOffset - 7, "", rightRequiredOffset, "", b80);
  
  if (truth) {
    int rightPadding = b80Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s%.100Lf\n", leftRequiredOffset - 3, "", rightPadding, "", b80Diff);
  }

  puts("\n");
}

void test128(int divisor, const char *truth) {
  int leftRequiredOffset = strlen("d128 - b128");
  _Float128 b128 = 1.0q / divisor;
  _Decimal128 d128 = 1.0dl / divisor;
  
  _Decimal128 overallDiff = d128 - (_Decimal128)b128; 
  int rightRequiredOffset = (int)(overallDiff < 0);
  
  _Float128 b128Diff;
  if (truth) {
    b128Diff = b128 - strtoflt128(truth, NULL);
    rightRequiredOffset |= (int)(b128Diff < 0);
  }

  _Decimal128 d128Diff;
  if (truth) {
    d128Diff = (_Decimal128)d128 - strtod128(truth, NULL);
    rightRequiredOffset |= (int)(d128Diff < 0);
  }
  
  // Need 1 if no minus and 2 if minus to ensure at least one space is printed
  rightRequiredOffset++;
  
  int expLen = printf("1.0 / %i", divisor);
  printf("%*s=%*s", leftRequiredOffset - expLen + 1, "", rightRequiredOffset, "");
  if (truth) {
    printf("%.121s\n\n", truth);
  } else {
    printf("Unknown\n\n");
  }
  
  int b128Len = printf("binary128");
  printf("%*s=%*s", leftRequiredOffset - b128Len + 1, "", rightRequiredOffset, "");
  printfloat("%.120Qf", b128);
 
  if (truth) {
    int rightPadding = b128Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s", leftRequiredOffset - 3, "", rightPadding, "");
    printfloat("%.120Qf", b128Diff);
  }
  putchar('\n');

  int d128Len = printf("decimal128");
  printf("%*s=%*s", leftRequiredOffset - d128Len + 1, "", rightRequiredOffset, "");
  printfloat("%.120f", d128);
  
  if (truth) {
    int rightPadding = d128Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s", leftRequiredOffset - 3, "", rightPadding, "");
    printfloat("%.120f", d128Diff);
  }
  putchar('\n');
  
  printf("d128 - b128 ≈ ", "");
  printfloat("%.120f", overallDiff);
  
  puts("\n");
}

int main() {
  puts("32 bit");
  test32(10, TEN);
  test32(7, SEVEN);
  test32(3, THREE);
  
  puts("\n64 bit");
  test64(10, TEN);
  test64(7, SEVEN);
  test64(3, THREE);
  
  puts("\n80 bit");
  test80(10, TEN);
  test80(7, SEVEN);
  test80(3, THREE);
  
  puts("\n128 bit");
  test128(10, TEN);
  test128(7, SEVEN);
  test128(3, THREE);
  
  return 0;
}
