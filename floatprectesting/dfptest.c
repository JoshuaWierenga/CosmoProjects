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

// TODO: Find out why manual null termination is needed for cosmo
#define printdec(type, strfrom, fstr, val) \
{\
  type dec = val;\
  int size = strfrom(NULL, 0, fstr, dec);\
  char str[size + 1];\
  strfrom(str, size, fstr, dec);\
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
  printdec(_Decimal32, strfromd32, "%.40f", d32);
  
  if (truth) {
    int rightPadding = d32Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s", leftRequiredOffset - 3, "", rightPadding, "");
    printdec(_Decimal128, strfromd128, "%.40f", d32Diff);
  }
  putchar('\n');
  
  printf("d32 - b32%*s≈ ", leftRequiredOffset - 8, "");
  printdec(_Decimal128, strfromd128, "%.40f", overallDiff);
  
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
  printdec(_Decimal64, strfromd64, "%.80f", d64);
  
  if (truth) {
    int rightPadding = d64Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s", leftRequiredOffset - 3, "", rightPadding, "");
    printdec(_Decimal128, strfromd128, "%.80f", d64Diff);
  }
  putchar('\n');
  
  printf("d64 - b64%*s≈ ", leftRequiredOffset - 8, "");
  printdec(_Decimal128, strfromd128, "%.80f", overallDiff);
  
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

void unfinishedtest128(int divisor, const char *truth) {
  int leftRequiredOffset = strlen("decimal128");
   __float128 b128 = 1.0l / divisor;
  _Decimal128 d128 = 1.0dl / divisor;
  
  _Decimal128 overallDiff = (_Decimal128)d128 - (_Decimal128)b128; 
  int rightRequiredOffset = (int)(overallDiff < 0);
  
  long double b128Diff;
  if (truth) {
    b128Diff = (long double)b128 - strtold(truth, NULL);
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
    printf("%.122s\n\n", truth);
  } else {
    printf("Unknown\n\n");
  }
  
  printf("binary128%*s=%*s%.120f\n", leftRequiredOffset - 7, "", rightRequiredOffset, "", b128);
  
  if (truth) {
    int rightPadding = b128Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s%.120Lf\n", leftRequiredOffset - 3, "", rightPadding, "", b128Diff);
  }
  putchar('\n');

  printf("decimal128 =%*s", rightRequiredOffset, "");
  printdec(_Decimal128, strfromd128, "%.120f", d128);
  
  if (truth) {
    int rightPadding = d128Diff < 0 ? 1 : rightRequiredOffset;
    printf("diff%*s≈%*s", leftRequiredOffset - 3, "", rightPadding, "");
    printdec(_Decimal128, strfromd128, "%.120f", d128Diff);
  }
  putchar('\n');
  
  printf("d128 - b128%*s≈ ", leftRequiredOffset - 8, "");
  printdec(_Decimal128, strfromd128, "%.120f", overallDiff);
  
  puts("\n");
}

void test128(int divisor, const char *truth) {
#ifdef __GLIBC__
    __float128 b128 = 1.0q / divisor;
    int bsize = quadmath_snprintf(NULL, 0, "%.120Qf", b128);
    char sb128[bsize + 1];
    quadmath_snprintf(sb128, bsize, "%.120Qf", b128);
    printf("1.0q  / %i = %s\n", divisor, sb128);
#else
    printf("1.0q  / %i = NOT SUPPORTED ON COSMO\n", divisor);
#endif

    _Decimal128 d128 = 1.0dl / divisor;
    int dsize = strfromd128(NULL, 0, "%.120f", d128);
    char sd128[dsize + 1];
    strfromd128(sd128, dsize, "%.120f", d128);
    sd128[dsize] = '\0';
    printf("1.0ld / %i = %s\n\n", divisor, sd128);
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
  //
  //for (int i = 0; i < 20; i++) {
  //  printf("142857");
  //}
  //putchar('\n');

  return 0;
}
