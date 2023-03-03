#include "lifescalar.h"
//#define USE_MODULUS

constexpr unsigned int CELL_ON = 0xFF000000;
constexpr unsigned int CELL_OFF = 0xFFFFFFFF;
constexpr unsigned int CELL_ON_RED = 0xFFFF0000;
constexpr unsigned int CELL_ON_BLUE = 0xFF0000FF;

static inline bool IsCellOn(unsigned int v)
{
    return (v ^ CELL_OFF);
}


/* Mark Garrett's implementation, converted for framebuffer pixels */
/*
  All of this is to determine the state of a single cell. The state of this cell
  for the next frame is not taken into account by the computation for the
  present frame of adjacent cells.

  Returns true if state of cell has changed.
 */
bool life(
    int *input,
    int* output,
    int i,
    const int height,
    const int width)
{
    int row = i / width;
    int col = i - (row * width);

    int rowUp = i - width;
    int rowDown = i + width;

    bool outOfBounds = (row <= 0) || (row >= height) || (col <= 0) || (col >= width);

    if (outOfBounds)
    {
        output[i] = CELL_OFF;
    }
    else
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

    return output[i] != input[i];
}

bool life(
    int *input,
    int* output,
    int i,
    const int height,
    const int width, const int shiftamt)
{
	int row = i >> shiftamt;
    int col = i - (row * width);

    int rowUp = i - width;
    int rowDown = i + width;

    bool outOfBounds = (row <= 0) || (row >= height) || (col <= 0) || (col >= width);

    if (outOfBounds)
    {
        output[i] = CELL_OFF;
    }
    else
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

    return output[i] != input[i];
}
