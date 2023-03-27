#include "lifescalar.h"
//#define USE_MODULUS

static inline bool IsCellOn(unsigned int v)
{
    return (v ^ CELL_OFF);
}

void life(
    const int *input,
    int* output,
    int i,
    const int height,
    const int width)
{
    int row = i / width;
    int col = i - (row * width);

    int rowUp = i - width;
    int rowDown = i + width;

    bool outOfBounds = (row <= 0) || (row >= height - 1) || (col <= 0) || (col >= width) ;

    if (!outOfBounds)
    {
        int neighbours = IsCellOn(input[rowUp-1]) + IsCellOn(input[rowUp]) + IsCellOn(input[rowUp+1]);
        neighbours += IsCellOn(input[i-1]) + IsCellOn(input[i+1]);
        neighbours += IsCellOn(input[rowDown-1]) + IsCellOn(input[rowDown]) + IsCellOn(input[rowDown+1]);
        if (neighbours == 3 || (IsCellOn(input[i]) && neighbours == 2))
        {
            output[i] = IsCellOn(input[i]) ? CELL_ON : CELL_ON_RED;
        }
        else
        {
            output[i] = CELL_OFF;
        }
    }
    else
    {
        output[i] = CELL_OFF;
    }
}
