// clang-format off
#include "libc/inttypes.h"
#include "libc/fmt/conv.h"
#include "libc/mem/mem.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/x/x.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"
#include "third_party/aoc/utilities/dynamicarray/ary.h"

#define NumberOfTrailingKnots 9

char directionChars[] = "LRUD";

struct GridSquare
{
    int_fast16_t X;
    int_fast16_t Y;
};

struct array_GridSquare ary(struct GridSquare*);

void moveHead(char direction, int_fast16_t* currentHeadX, int_fast16_t* currentHeadY)
{
    switch(direction)
    {
        case 'L':
            if (*currentHeadX == INT_FAST16_MIN)
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            (*currentHeadX)--;
            break;
        case 'R':
            if (*currentHeadX == INT_FAST16_MAX)
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            (*currentHeadX)++;
            break;
        case 'U':
            if (*currentHeadY == INT_FAST16_MAX)
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            (*currentHeadY)++;
            break;
        case 'D':
            if (*currentHeadY == INT_FAST16_MIN)
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            (*currentHeadY)--;
            break;
    }
}

void updateTrailing(int_fast16_t* currentTrailingX, int_fast16_t* currentTrailingY, int_fast16_t* previousTrailingX, int_fast16_t* previousTrailingY)
{
    bool currentlyLeft = *currentTrailingX + 1 < *previousTrailingX;
    bool currentlyRight = *currentTrailingX - 1 > *previousTrailingX;
    bool currentlyDown = *currentTrailingY + 1 < *previousTrailingY;
    bool currentlyUp = *currentTrailingY - 1 > *previousTrailingY;
    bool currentlyWrongX = currentlyLeft || currentlyRight;
    bool currentlyWrongY = currentlyDown || currentlyUp;
    
    if (currentlyLeft || (*currentTrailingX < *previousTrailingX && currentlyWrongY))
    {
        (*currentTrailingX)++;
    }
    else if (currentlyRight || (*currentTrailingX > *previousTrailingX && currentlyWrongY))
    {
        (*currentTrailingX)--;
    }
    
    if (currentlyDown || (*currentTrailingY < *previousTrailingY && currentlyWrongX))
    {
        (*currentTrailingY)++;
    }
    else if (currentlyUp || (*currentTrailingY > *previousTrailingY && currentlyWrongX))
    {
        (*currentTrailingY)--;
    }
}

bool alreadyVisited(struct array_GridSquare previousSquares, struct GridSquare* newSquare)
{
    for (size_t i = 0; i < previousSquares.len; i++)
    {
        struct GridSquare* previousSquare = previousSquares.buf[i];
        if (previousSquare->X == newSquare->X && previousSquare->Y == newSquare->Y)
        {
            return true;
        }
    }
    
    return false;
}

//TODO Support performance testing
void task9()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Big enough for the direction char, a space, up to 2 chars for movement amount, new line and terminating char
    char chunk[6];
    
    struct array_GridSquare tailSquares[NumberOfTrailingKnots];
    // Just used the number of moves in my input, is likely to be quite a bit smaller
    for (size_t i = 0; i < NumberOfTrailingKnots; i++)
    {
        ary_init(&tailSquares[i], NumberOfTrailingKnots);
    }
    
    // Assuming grid is within (INT_FAST16_MIN, INT_FAST16_MIN) to (INT_FAST16_MAX, INT_FAST16_MAX)
    int_fast16_t minX = 0;
    int_fast16_t minY = 0;
    int_fast16_t maxX = 0;
    int_fast16_t maxY = 0;
    // For convience I have flipped Y with up increasing and down decreasing it, the sample starts in the bottom left but I have the top left here
    int_fast16_t currentKnotX[NumberOfTrailingKnots + 1] = {0};
    int_fast16_t currentKnotY[NumberOfTrailingKnots + 1] = {0};
    
    while (fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        char directionChar = chunk[0];

        char* integerEndPtr;
        uint_fast8_t movementAmount = strtoul(chunk + 2, &integerEndPtr, 10);

        if (strchr(directionChars, directionChar) == NULL || chunk[1] != ' ' ||
            integerEndPtr == chunk || integerEndPtr == NULL || (integerEndPtr[0] != '\n' && integerEndPtr[0] != '\0'))
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        for (uint_fast8_t i = movementAmount; i > 0; i--)
        {
            moveHead(directionChar, currentKnotX, currentKnotY);
            for (size_t i = 0; i < NumberOfTrailingKnots; i++)
            {
                updateTrailing(currentKnotX + i + 1, currentKnotY + i + 1, currentKnotX + i, currentKnotY + i);
                
                struct GridSquare* newTailPosition = malloc(sizeof *newTailPosition);
                newTailPosition->X = currentKnotX[i + 1];
                newTailPosition->Y = currentKnotY[i + 1];
                
                if (!alreadyVisited(tailSquares[i], newTailPosition))
                {
                    ary_push(tailSquares + i, newTailPosition);
                }
                else
                {
                    free(newTailPosition);
                }
            }
        }
        
        minX = MIN(minX, *currentKnotX);
        minY = MIN(minY, *currentKnotY);
        maxX = MAX(maxX, *currentKnotX);
        maxY = MAX(maxY, *currentKnotY);
    }
    
    size_t firstKnotVisitedSquareCount = tailSquares[0].len;
    size_t lastKnotVisitedSquareCount = tailSquares[NumberOfTrailingKnots - 1].len;
    
    /*printf("\nGrid Size: (%i, %i)\n", maxX - minX + 1, maxY - minY + 1);
    printf("Bottom left (%i, %i), Top right (%i, %i)\n\n", minX, minY, maxX, maxY);
   
    struct GridSquare square;

    for (int_fast16_t y = maxY; y >= minY; y--)
    {
        for(int_fast16_t x = minX; x <= maxX; x++)
        {
            square.X = x;
            square.Y = y;
    
            bool visited = alreadyVisited(tailSquares[NumberOfTrailingKnots - 1], &square);
            char* visitedChar = visited ? "#" : ".";
            
            printf((x == 0 && y == 0) ? "\033[1;31ms\033[0m" : visitedChar);
            putchar(' ');
        }
        putchar('\n');
    }*/

    for (size_t i = 0; i < NumberOfTrailingKnots; i++)
    {
        for (size_t j = 0; j < tailSquares[i].len; j++)
        {
            free(tailSquares[i].buf[j]);
        }
        ary_release(tailSquares + i);
    }
    fclose(fp);
    
    printf("Part 1: Number of squares visited by the first knot:% *s%zu\n", MAX(1, LENGTH(NumberOfTrailingKnots) - 2), "", firstKnotVisitedSquareCount);
    printf("Part 2: Number of squares visited by the %zuth knot:% *s%zu\n", NumberOfTrailingKnots, MAX(1, 4 - LENGTH(NumberOfTrailingKnots)), "", lastKnotVisitedSquareCount);
}
