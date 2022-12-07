// clang-format off
#include "libc/inttypes.h"
#include "libc/mem/gc.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/x/x.h"
#include "libc/x/xasprintf.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"
#include "third_party/aoc/utilities/dynamicarray/ary.h"

#define FileSystemSize 70000000

enum FileSystemItemType
{
    File,
    Directory
};

struct FileSystemItem
{
    enum FileSystemItemType Type;
    char* Name;
    uint_fast32_t Size; // For files this is just their size, for directories this is the sum of their children's size
    struct FileSystemItem* Parent;
    struct ary(struct FileSystemItem*) Children;
};

struct FileSystemItem* FindFSItem(struct FileSystemItem* parent, enum FileSystemItemType type, const char* name)
{
    if (parent->Type != Directory)
    {
        return NULL;
    }
    
    for (size_t i = 0; i < parent->Children.len; i++)
    {
        struct FileSystemItem* child = parent->Children.buf[i];
        
        if (child->Type == type && strcmp(child->Name, name) == 0)
        {
            return child;
        }
    }
    
    return NULL;
}

uint_fast32_t CalcuateDirectorySizes(struct FileSystemItem* directory)
{
    if (directory->Size != 0)
    {
        return directory->Size;
    }
    
    for (size_t i = 0; i < directory->Children.len; i++)
    {
        directory->Size += CalcuateDirectorySizes(directory->Children.buf[i]);
    }
    
    return directory->Size;
}

uint_fast32_t SumSmallDirectorySizes(struct FileSystemItem* directory, uint_fast32_t maxSize)
{
    if (directory->Type != Directory)
    {
        return 0;
    }
    
    uint_fast32_t total = 0;
    
    if (directory->Size <= maxSize)
    {
        total += directory->Size;
    }
    
    for (size_t i = 0; i < directory->Children.len; i++)
    {
        total += SumSmallDirectorySizes(directory->Children.buf[i], maxSize);
    }
    
    return total;
}

uint_fast32_t FindClosestDirectorySize(struct FileSystemItem* directory, uint_fast32_t value)
{
    if (directory->Type != Directory)
    {
        return 0;
    }
    
    uint_fast32_t closestSize = FileSystemSize;
    if (directory->Size >= value)
    {
        closestSize = directory->Size;
    }
    
    for (size_t i = 0; i < directory->Children.len; i++)
    {
        uint_fast32_t newSize = FindClosestDirectorySize(directory->Children.buf[i], value);
        if (newSize >= value && newSize < closestSize)
        {
            closestSize = newSize;
        }
    }
    
    return closestSize;
}

void FreeFS(struct FileSystemItem* item)
{
    //free(name); // TODO Remove _gc usage first
    
    if (item->Type == File)
    {
        return;
    }
    
    for (size_t i = 0; i < item->Children.len; i++)
    {
        struct FileSystemItem* child = item->Children.buf[i];
        
        FreeFS(child);
        //free(child); // TODO Remove _gc usage first
    }
    
    ary_release(&item->Children);
}

void PrintFS(struct FileSystemItem directory, uint_fast8_t depth, bool matchSample)
{
    if (matchSample)
    {
        printf("% *s- %s (dir)\n", 2 * depth, "", directory.Name);
    }
    else
    {
        printf("% *s- %s (dir, total size=%" PRIuFAST32 ")\n", 2 * depth, "", directory.Name, directory.Size);
    }
    
    for (size_t i = 0; i < directory.Children.len; i++)
    {
        struct FileSystemItem* child = directory.Children.buf[i];
        
        if (child->Type == File)
        {
            printf("% *s- %s (file, size=%" PRIuFAST32 ")\n", 2 * (depth + 1), "", child->Name, child->Size);
        }
        else
        {
            PrintFS(*child, depth + 1, matchSample);
        }
    }
}

//TODO Support performance testing
void task7()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Randomly picked size, enough for 126 chars, new line and terminating char
    char chunk[128];
    
    bool processingLS = false;
    struct FileSystemItem* currentDirectory = NULL;
    
    struct FileSystemItem root = { Directory, "/", 0, NULL };
    // Guess at number of children, will expand if not big enough
    ary_init(&root.Children, 32);
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        // Process new command
        if (chunk[0] == '$' && chunk[1] == ' ')
        {
            if (chunk[2] == 'l' && chunk[3] == 's')
            {
                processingLS = true;
                continue;
            }
            
            processingLS = false;
            
            if (chunk[2] != 'c' || chunk[3] != 'd' || chunk[4] != ' ')
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            if (chunk[5] == '/' && chunk[6] == '\n')
            {
                currentDirectory = &root;
                continue;
            }
            
            if (chunk[5] == '.' && chunk[6] == '.' && chunk[7] == '\n')
            {
                currentDirectory = currentDirectory->Parent;
                continue;
            }
            
            size_t argumentLength = strlen(chunk + 5);
            if (chunk[argumentLength + 4] == '\n')
            {
                argumentLength--;
            }
            
            char* directoryName = xasprintf("%.*s", argumentLength, chunk + 5);
            struct FileSystemItem* foundDirectory = FindFSItem(currentDirectory, Directory, directoryName);
            if (foundDirectory == NULL)
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            currentDirectory = foundDirectory;
            
            free(directoryName);
            continue;
        }
        
        if (!processingLS || currentDirectory == NULL)
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        // TODO Merge later parts
        if (chunk[0] == 'd' && chunk[1] == 'i' && chunk[2] == 'r' && chunk[3] == ' ')
        {
            size_t argumentLength = strlen(chunk + 4);
            if (chunk[argumentLength + 3] == '\n')
            {
                argumentLength--;
            }
            
            char* directoryName = xasprintf("%.*s", argumentLength, chunk + 4);
            struct FileSystemItem* foundDirectory = FindFSItem(currentDirectory, Directory, directoryName);
            
            if (foundDirectory != NULL)
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            // TODO Consider not using _gc with testing as it only frees after the function ends
            // which can result in lots of unused allocated memory until that happens
            // Testing system currently runs 100000 times, that is a lot of used memory
            struct FileSystemItem* newDirectory = _gc(xmalloc(sizeof *newDirectory));
            newDirectory->Type = Directory;
            newDirectory->Name = _gc(directoryName);
            newDirectory->Size = 0;
            newDirectory->Parent = currentDirectory;
            // Guess at number of children, will expand if not big enough
            ary_init(&newDirectory->Children, 8);
            
            ary_push(&currentDirectory->Children, newDirectory);
        }
        else
        {
            char* integerEndPtr;
            uint_fast32_t fileSize = strtoul(chunk, &integerEndPtr, 10);
            if (integerEndPtr == chunk || integerEndPtr == NULL || integerEndPtr[0] != ' ' ||
            integerEndPtr[1] == '\n' || integerEndPtr[1] == '\0') // No chars were read, at end of string or doesn't resemble required format
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            integerEndPtr += 1; // Account for space
            size_t argumentLength = strlen(integerEndPtr);
            if (integerEndPtr[argumentLength - 1] == '\n')
            {
                argumentLength--;
            }
            
            char* fileName = xasprintf("%.*s", argumentLength, integerEndPtr);
            struct FileSystemItem* foundFile = FindFSItem(currentDirectory, File, fileName);
            
            if (foundFile != NULL)
            {
                perror("Unable to parse input file");
                exit(1);
            }

            // TODO Consider not using _gc with testing as it only frees after the function ends
            // which can result in lots of unused allocated memory until that happens
            // Testing system currently runs 100000 times, that is a lot of used memory
            struct FileSystemItem* newFile = _gc(xmalloc(sizeof *newFile));
            newFile->Type = File;
            newFile->Name = _gc(fileName);
            newFile->Size = fileSize;
            newFile->Parent = currentDirectory;
            
            ary_push(&currentDirectory->Children, newFile);
        }
    }
    
    CalcuateDirectorySizes(&root);
    //PrintFSFull(root, 0);
    
    uint_fast32_t smallestDirectorySum = SumSmallDirectorySizes(&root, 100000);
    
    uint_fast32_t requiredAmount = 30000000 - (FileSystemSize - root.Size);
    uint_fast32_t closestDirectorySize = FindClosestDirectorySize(&root, requiredAmount);
    
    FreeFS(&root);
    fclose(fp);
    
    printf("Part 1: Sum of directories with total size of at most 100000:   %" PRIuFAST32 "\n", smallestDirectorySum);
    printf("Part 2: Size of smallest directory larger than %" PRIuFAST32 ":         %" PRIuFAST32 "\n",requiredAmount, closestDirectorySize);
}
