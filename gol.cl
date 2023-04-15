/* This is kernel used to represent each cell in the Game of Life */

#define CELL_ON 0xFF000000
#define CELL_OFF 0xFFFFFFFF
#define CELL_ON_RED 0xFF00FF00

bool IsCellOn(unsigned int v)
{
    return (v ^ CELL_OFF);
}

__kernel void life(
   global int * input,
   global int * output,
   const unsigned int height,
   const unsigned int width)
{
   int i = get_global_id(0);
   int row = i / width;
   int col = i - (row * width);
   int rowUp = i - width;
   int rowDown = i + width;

   bool outOfBounds = (row <= 0) || (row >= height - 1) || (col <= 0) || (col >= width);


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
