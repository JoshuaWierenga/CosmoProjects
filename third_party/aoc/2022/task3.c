// clang-format off
#include "libc/inttypes.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/aoc/utilities/hashmap/src/cmap.h"
#include "third_party/aoc/utilities/aocdatamanagement.c"

char* year = "2022";
char* inputFileName = "task3.data";

uint_fast8_t getItemPriority(char item)
{
    if (item >= 'A' && item <= 'Z')
    {
        return item - 'A' + 27;
    }
    
    if (item >= 'a' && item <= 'z')
    {
        return item - 'a' + 1;
    }
    
    perror("Unable to open input file");
    exit(1);
}

void task3()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Not sure on length so assume quite large, this allows 126 chars, a new line and the string terminator
    char chunk[128];
    
    uint_fast16_t prioritySumPart1 = 0;
    uint_fast16_t groupPrioritySumPart2 = 0;
    // Stores the number of group members that have the given item in their rucksacks
    // For performance reasons, rather than wiping the list between groups, it just
    // sets the number of group members with the item to 0
    map_t(char, uint_fast8_t) groupItems;
    map_stdinit(&groupItems);

    for (uint_fast8_t i = 0; fgets(chunk, sizeof(chunk), fp) != NULL; i++)
    {
        char* lineEnd = strchr(chunk, '\n');
        if (lineEnd == NULL)
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        uint_fast8_t length = lineEnd - chunk;
        if (length % 2 != 0)
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        uint_fast8_t compartmentLength = length / 2;
        // Do not need the value currently but getting a hashset type as well felt overkill
        // TODO Reuse map
        map_t(char, uint_least8_t) compartment1Items;
        map_stdinit(&compartment1Items);
        
        //TODO Merge loops
        for (int j = 0; j < compartmentLength; j++)
        {
            char currentItem = chunk[j];
            map_set(&compartment1Items, currentItem, 0);
            
            uint_fast8_t* currentGroupCount = map_get(&groupItems, currentItem);
            if ((currentGroupCount == NULL && i == 0) || (currentGroupCount != NULL && i == *currentGroupCount))
            {
                map_set(&groupItems, currentItem, i + 1);
            }
        }
        
        char misplacedItem = '\0';
        
        for (int j = 0; j < compartmentLength; j++)
        {
            char currentItem = chunk[compartmentLength + j];
            if (misplacedItem == '\0' && map_get(&compartment1Items, currentItem) != NULL)
            {
                misplacedItem = currentItem;
            }
            
            uint_fast8_t* currentGroupCount = map_get(&groupItems, currentItem);
            if ((currentGroupCount == NULL && i == 0) || (currentGroupCount != NULL && i == *currentGroupCount))
            {
                map_set(&groupItems, currentItem, i + 1);
            }
        }
  
        if (misplacedItem == '\0')
        {
            perror("Unable to parse input file");
            exit(1);
        }

        prioritySumPart1 += getItemPriority(misplacedItem);
                
        if (i == 2)
        {
            i = -1;

            char groupItem = '\0';
            char loopItem;
            map_iter_t iter = map_iter(&groupItems);

            while (map_next(&groupItems, &iter, &loopItem))
            {
                if (groupItem == '\0' && *map_get(&groupItems, loopItem) == 3)
                {
                    groupItem = loopItem;
                }
        
                map_set(&groupItems, loopItem, 0);
            }
            
            if (groupItem == '\0')
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            groupPrioritySumPart2 += getItemPriority(groupItem);
        }
        
        map_delete(&compartment1Items);
    }
    
    printf("Part 1: Sum of priorities of misplaced items: %" PRIdFAST16 "\n", prioritySumPart1);
    printf("Part 2: Sum of priorities of group badges:    %" PRIdFAST16 "\n", groupPrioritySumPart2);

    map_delete(&groupItems);
    fclose(fp);
}
