#ifdef __COSMOPOLITAN__
#include <cosmo.h>
#endif
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <testlib.h>
#if defined(__linux__) || defined(__COSMOPOLITAN__)
#include <dlfcn.h>
#endif

#ifdef COSMO2
char windowsLibPath[] = "/tmp/XXXXXX.dll";
char linuxLibPath[] = "/tmp/XXXXXX.so";
int windowsLibPathSuffixLen = 4;
int linuxLibPathSuffixLen = 3;
#endif /* COSMO2 */

#if defined(__linux__ )|| defined(__COSMOPOLITAN__)
#define openlib(path, flags) dlopen(path, flags)
#define openliberror "%s\n", dlerror()
#define openfunc(lib, name) dlsym(lib, name)
#define openfuncerror openliberror
#define closelib(lib) dlclose(lib)
#else
int64_t LoadLibraryW(const wchar_t *lpLibFileName);
void *GetProcAddress(int64_t hModule, const char *lpProcName);
int32_t FreeLibrary(int64_t hLibModule);  

#define openlib(path, flags) (void *)LoadLibraryW(path)
#define openliberror "Failed to load library\n"
#define openfunc(lib, name) GetProcAddress((int64_t)lib, name)
#define openfuncerror "Failed to load function\n"
#define closelib(lib) FreeLibrary((int64_t)lib)
#endif

typedef int32_t (*ADDFUNC)(int32_t, int32_t);

int32_t add(int32_t a, int32_t b) {
#ifdef __COSMOPOLITAN__
  char *libPath;
#ifdef COSMO2
  int libPathSuffixLen;
  char *zipPath;
  if (IsLinux()) {
    puts("Cosmo2 Linux");
    libPath = linuxLibPath;
    libPathSuffixLen = linuxLibPathSuffixLen;
    zipPath = "/zip/lib/libtestlib.so";
  } else if (IsWindows()) {
    puts("Cosmo2 Windows");
    libPath = windowsLibPath;
    libPathSuffixLen = windowsLibPathSuffixLen;
    zipPath = "/zip/lib/libtestlib.dll";
  } else {
    fprintf(stderr, "OS not supported");
    exit(1);
  }
    
  // From libc/testlib/extract.c
  int zipFd, libFd;
  if ((zipFd = open(zipPath, O_RDONLY)) == -1) {
    perror(zipPath);
    exit(1);
  }
  if ((libFd = openatemp(AT_FDCWD, libPath, libPathSuffixLen, 0, 0)) == -1) {
    perror(libPath);
    exit(1);
  }
  if (copyfd(zipFd, libFd, -1) == -1) {
    perror(zipPath);
    exit(1);
  }
  if (close(libFd)) {
    perror(libPath);
    exit(1);
  }
  if (close(zipFd)) {
    perror(zipPath);
    exit(1);
  }
#else
  if (IsLinux()) {
    puts("Cosmo Linux");
    libPath = "libtestlib.so";
  } else if (IsWindows()) {
    puts("Cosmo Windows");
    libPath = "libtestlib.dll";
  } else {
    fprintf(stderr, "OS not supported");
    exit(1);
  }
#endif /* COSMO2 */
#elif defined(__linux__)
  puts("glibc/musl Linux");
  char libPath[] = "libtestlib.so";
#elif defined(_WIN32)
  puts("crt Windows");
  wchar_t libPath[] = L"libtestlib.dll";
#endif /* __COSMOPOLITAN__, __linux__, _WIN32 */

#ifdef _WIN32
  printf("PATH: %S\n", libPath);
#else
  printf("PATH: %s\n", libPath);
#endif /* _WIN32 */

  void *testlib = openlib(libPath, RTLD_LAZY);
  if (!testlib) {
    fprintf(stderr, openliberror);
    exit(1);
  }
  
  ADDFUNC pAdd = (ADDFUNC)openfunc(testlib, "add");
  if (!pAdd) {
    fprintf(stderr, openfuncerror);
    exit(1);
  }
  
  int32_t result = pAdd(a, b);
  
  closelib(testlib);
  return result;
}
#if !defined(__linux__) && !defined(__COSMOPOLITAN__) && !defined(_WIN32)
#error OS not supported
#endif /* !__linux__ && !__COSMOPOLITAN__ && !_WIN32 */

int main(void) {
  int32_t a = 1;
  int32_t b = 5;
  int32_t result = add(a, b);
  printf("%" PRId32 " + %" PRId32 " = %" PRId32 "\n", a, b, result);
  return 0;
}
