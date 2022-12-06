// clang-format off
#include "libc/inttypes.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/aoc/utilities/aocdatamanagement.c"

#define startOfPacketLength 4
#define startOfMessageLength 14

void task6()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Could improve performance by reading more than 1 chars at a time but this works for now
    char chunk[2];
    
    uint_fast16_t currentIndex = 0;
    uint_fast16_t startOfPacketIndex = 0;
    uint_fast16_t startOfMessageIndex = 0;
    uint_fast16_t badIndex = 0;
    
    char data[startOfMessageLength + 1];
    memset(data, '0', startOfMessageLength);
    data[startOfMessageLength] = '\0';
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        if (chunk[0] == '\n')
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        memmove(data, data + 1, (startOfMessageLength - 1) * sizeof *data);
        
        // TODO reverse data so that new chars go on the left, then can just use strchr
        // without this mess
        // Work around strrchr finding chunk[0] itself if it is added before hand
        // also need to finish the shift first so just set the new position to a junk value
        data[startOfMessageLength - 1] = '0';
        char* lastDuplicatePos = strrchr(data, chunk[0]);
        data[startOfMessageLength - 1] = chunk[0];
        
        if (lastDuplicatePos != NULL && lastDuplicatePos != data + startOfMessageLength - 1)
        {
            uint_fast16_t lastDuplicateIndex = currentIndex - strlen(lastDuplicatePos) + 1;
            if (badIndex < lastDuplicateIndex)
            {
                badIndex = lastDuplicateIndex;
            }
        }
        
        if (startOfPacketIndex == 0 && currentIndex - badIndex >= startOfPacketLength)
        {
            startOfPacketIndex = currentIndex + 1;
        }
        
        if (currentIndex - badIndex >= startOfMessageLength)
        {
            startOfMessageIndex = currentIndex + 1;
            break;
        }
        
        currentIndex++;
    }
    
    printf("Part 1: Number of pairs with full assignment overlap: %" PRIdFAST16 "\n", startOfPacketIndex);
    printf("Part 2: Number of pairs with any assignment overlap:  %" PRIdFAST16 "\n", startOfMessageIndex);
    
    fclose(fp);
}
