// clang-format off
#include "libc/inttypes.h"
#include "libc/mem/mem.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/x/x.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"

void setGridRow(uint_fast8_t* grid, uint_fast16_t gridSize, uint_fast16_t row, char* rowData)
{
    if (grid == NULL || gridSize == 0 || rowData == NULL)
    {
        perror("Unable to parse input file");
        exit(1);
    }
    
    for (uint_fast8_t x = 0; x < gridSize; x++)
    {
        char treeHeight = *rowData++;
        
        if (treeHeight < '0' || treeHeight > '9')
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        grid[row * gridSize + x] = treeHeight - '0';
    }
}

uint_fast8_t getGridData(uint_fast8_t* grid, uint_fast16_t gridSize, uint_fast16_t row, uint_fast16_t column)
{
    return grid[row * gridSize + column];
}

// Very bad method since it is O(n^3) but it does work
uint_fast16_t GetVisibleCount(uint_fast8_t* grid, uint_fast16_t gridSize)
{
    if (grid == NULL || gridSize == 0)
    {
        perror("Unable to parse input file");
        exit(1);
    }
    
    uint_fast16_t visibleTreeCount = 0;
    
    for (uint_fast8_t y = 0; y < gridSize; y++)
    {
        uint_fast8_t highestSeenLeft = 0;
        
        for (uint_fast8_t x = 0; x < gridSize; x++)
        {
            uint_fast8_t currentTree = getGridData(grid, gridSize, y, x);
            bool visibleFromLeft = highestSeenLeft < currentTree;
            bool visibleFromRight = true;
            bool visibleFromTop = true;
            bool visibleFromBottom = true;
            
            highestSeenLeft = MAX(highestSeenLeft, currentTree);
            
            if (x == 0 || visibleFromLeft)
            {
                visibleTreeCount++;
                continue;
            }
            
            for (uint_fast8_t x2 = x + 1; visibleFromRight && x2 < gridSize; x2++)
            {
                uint_fast8_t currentTree2 = getGridData(grid, gridSize, y, x2);
                visibleFromRight = currentTree2 < currentTree;
            }
            
            for (uint_fast8_t y2 = 0; !visibleFromRight && visibleFromTop && y2 < y; y2++)
            {
                uint_fast8_t currentTree2 = getGridData(grid, gridSize, y2, x);
                visibleFromTop = currentTree2 < currentTree;
            }
            
            for (uint_fast8_t y2 = y + 1; !visibleFromRight && !visibleFromTop && visibleFromBottom && y2 < gridSize; y2++)
            {
                uint_fast8_t currentTree2 = getGridData(grid, gridSize, y2, x);
                visibleFromBottom = currentTree2 < currentTree;
            }
            
            visibleTreeCount += visibleFromRight || visibleFromTop || visibleFromBottom;
        }
    }
    
    return visibleTreeCount;
}

// Very bad method since it is O(n^3) but it does work
uint_fast32_t GetScenicScore(uint_fast8_t* grid, uint_fast16_t gridSize)
{
    uint_fast32_t maxScenicScore = 0;
    
    for (uint_fast8_t y = 0; y < gridSize; y++)
    {
        for (uint_fast8_t x = 0; x < gridSize; x++)
        {
            uint_fast8_t currentTree = getGridData(grid, gridSize, y, x);
            uint_fast16_t leftDistance = 0;
            uint_fast16_t rightDistance = 0;
            uint_fast16_t topDistance = 0;
            uint_fast16_t bottomDistance = 0;
            bool finishedLeft = false;
            bool finishedRight = false;
            bool finishedTop = false;
            bool finishedBottom = false;
            
            for (int_fast8_t x2 = x - 1; !finishedLeft && x2 >= 0; x2--)
            {
                uint_fast8_t currentTree2 = getGridData(grid, gridSize, y, x2);
                finishedLeft = currentTree2 >= currentTree;
                leftDistance++;
            }
            
            for (int_fast8_t x2 = x + 1; !finishedRight && x2 < gridSize; x2++)
            {
                uint_fast8_t currentTree2 = getGridData(grid, gridSize, y, x2);
                finishedRight = currentTree2 >= currentTree;
                rightDistance++;
            }
            
            for (int_fast8_t y2 = y - 1; !finishedTop && y2 >= 0; y2--)
            {
                uint_fast8_t currentTree2 = getGridData(grid, gridSize, y2, x);
                finishedTop = currentTree2 >= currentTree;
                topDistance++;
            }
            
            for (int_fast8_t y2 = y + 1; !finishedBottom && y2 < gridSize; y2++)
            {
                uint_fast8_t currentTree2 = getGridData(grid, gridSize, y2, x);
                finishedBottom = currentTree2 >= currentTree;
                bottomDistance++;
            }
            
            uint_fast16_t newScenicScore = leftDistance * rightDistance * topDistance * bottomDistance;
            maxScenicScore = MAX(maxScenicScore, newScenicScore);
        }
    }
    
    return maxScenicScore;
}

//TODO Support performance testing
void task8()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Randomly picked size, enough for 126 chars, new line and terminating char
    char chunk[128];
    
    // Assumes grid is square
    uint_fast16_t gridSize = 0;
    uint_fast8_t* grid = NULL;
    
    for (uint_fast16_t x = 0; fgets(chunk, sizeof(chunk), fp) != NULL; x++)
    {
        if (x == 0 && gridSize == 0)
        {    
            gridSize = strlen(chunk) - 1;
            if (chunk[gridSize] != '\n')
            {
                perror("Unable to parse input file");
                exit(1);
            }
            grid = xmalloc(gridSize * gridSize * sizeof *grid);
        }
        
        setGridRow(grid, gridSize, x, chunk);
    }
    
    uint_fast16_t visibleCount = GetVisibleCount(grid, gridSize);
    uint_fast32_t scenicScore = GetScenicScore(grid, gridSize);
    
    free(grid);
    fclose(fp);
    
    printf("Part 1: Number of trees visible from outside the grid:   %" PRIuFAST16 "\n", visibleCount);
    printf("Part 2: Highest scenic score for tree in grid:           %" PRIuFAST32 "\n", scenicScore);
}
