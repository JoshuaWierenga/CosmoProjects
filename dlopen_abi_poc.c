#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

// Alternative to https://github.com/JoshuaWierenga/CosmoGUI's wrapper generator to deal with ms and sysv abis.
// I have yet to actually test this with cosmo's dlopen so there may be an issue my testing missed.

//Only used for testing, IsWindows is a substitute for cosmo's IsWindows()
#if 1
#define ABI __attribute__((ms_abi))
volatile int IsWindows = 1;
#else
#define ABI __attribute__((sysv_abi))
volatile int IsWindows = 0;
#endif

// In native library with ABI implicitly determined by compiler
ABI int real_test1(int a, char *b) {
    if (a) return -1;
    printf("test: %s\n", b);
    return 5;
}

// In native library with ABI implicitly determined by compiler
ABI int real_test2() {
    if (IsWindows) {
        puts("On windows and using ms_abi");
    } else {
        puts("On !windows and using sysv_abi");
    }
    return 3;
}

// To be added to cosmo's dlfcn.h, should these be static?
#define cosmo_dldef(func) \
    __attribute__((ms_abi)) typeof(func) func##_ms;\
    __attribute__((sysv_abi)) typeof(func) func##_sysv
#define cosmo_dlcall(func, ...) (IsWindows ? func##_ms(__VA_ARGS__) : func##_sysv(__VA_ARGS__)) 

// To be added to cosmo's dlfcn.h, a cosmo_dlsym wrapper that handles abi by depending on cosmo_dldef
// Unlike cosmo_dlsym this takes the function name by definition instead of by string
// Returns the result of the cosmo_dlsym in case that is still needed
#define cosmo_dlsym2(h, func) ({\
    void *func = cosmo_dlsym(h, #func);\
    func##_ms = func;\
    func##_sysv = cosmo_dltramp(func);\
    func;\
})

// Only used for testing, is a substitute for the real cosmo_dlopen
void *cosmo_dlopen(const char *path, int mode) {
    return 0;
}

// Only used for testing, is a substitute for the real cosmo_dlsym
void *cosmo_dlsym(void *handle, const char *name) {
    if (strcmp("test1", name) == 0) {
        return real_test1;
    } else if (strcmp("test2", name) == 0) {
        return real_test2;
    }
    return 0; 
}

// Only used for testing, is a substitute for the real cosmo_dltramp
void *cosmo_dltramp(void *foreign_func) {
    return foreign_func;
}

// In library's header
extern int (*test1)(int, char *);
extern int (*test2)(void);

// In user code, defines global function pointers
// Used in the same way as the function typedef in dlsym/GetProcAddress code
cosmo_dldef(test1);
cosmo_dldef(test2);

// In user code, optional define to approximate regular dlsym/GetProcAddress usage
#define test1(...) cosmo_dlcall(test1, __VA_ARGS__)
//#define test2(...) cosmo_dlcall(test2, __VA_ARGS__)

int main(void) {
    void *h = cosmo_dlopen("hi", RTLD_LOCAL);
    cosmo_dlsym2(h, test1);
    cosmo_dlsym2(h, test2);
    test1(0, "Hello, World!");
    return cosmo_dlcall(test2);
}


