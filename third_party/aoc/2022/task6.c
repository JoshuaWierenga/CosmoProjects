// clang-format off
#include "libc/inttypes.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"

#define startOfPacketLength 4
#define startOfMessageLength 14

void task6()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Could improve performance by reading more than 1 chars at a time but this works for now
    char chunk[2];
    
    uint_fast16_t startOfPacketIndex = 0;
    uint_fast16_t startOfMessageIndex = 0;
    
    char data[startOfMessageLength + 1] = { 0 };
    
    startTesting();
    
    uint_fast16_t currentIndex = 0;
    uint_fast16_t badIndex = 0;
    
    rewind(fp);
    memset(data, '0', startOfMessageLength);
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        if (chunk[0] == '\n')
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        memmove(data + 1, data, (startOfMessageLength - 1) * sizeof *data);
        data[0] = chunk[0];
        
        char* lastDuplicatePos = strchr(data + 1, chunk[0]);
        if (lastDuplicatePos != NULL)
        {
            uint_fast16_t lastDuplicateIndex = currentIndex - startOfMessageLength + strlen(lastDuplicatePos);
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
    
    endTesting();
    fclose(fp);
    
    printf("Part 1: Number of characters before first start-of-packet marker:   %" PRIdFAST16 "\n", startOfPacketIndex);
    printf("Part 2: Number of characters before first start-of-message marker:  %" PRIdFAST16 "\n", startOfMessageIndex);
}
