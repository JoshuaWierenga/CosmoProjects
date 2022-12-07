#include "libc/mem/gc.h"
#include "libc/runtime/runtime.h"
#include "libc/str/str.h"
#include "libc/x/xasprintf.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"

// TODO Get this to work in different locations at runtime
FILE* faocdataopen(const char* sourcePath)
{
    size_t sourcePathNoExtLength = strrchr(sourcePath, '.') - sourcePath + 1;
    char* dataPath = _gc(xasprintf("%.*sdata", sourcePathNoExtLength, sourcePath));
    
    // Based on top example from https://solarianprogrammer.com/2019/04/03/c-programming-read-file-lines-fgets-getline-implement-portable-getline/
    // as the issues presented are not relevant here
    FILE* fp = fopen(dataPath, "r");
    if (fp == NULL)
    {
        perror("Unable to open input file");
        exit(1);
    }
    
    return fp;
}