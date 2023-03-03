#include "lifescalar.h"
//#define USE_MODULUS

constexpr unsigned int CELL_ON = 0xFF000000;
constexpr unsigned int CELL_OFF = 0xFFFFFFFF;

static inline bool IsCellOn(unsigned int v)
{
	return !(v ^ CELL_ON);
}


/* Mark Garrett's implementation, converted for framebuffer pixels*/
void life(
   int *input,
   int* output,
   int i,
   const int height,
   const int width)
{
   #ifndef USE_MODULUS
   int row = i / width;
   int col = i - (row * width);

   int rowUp = i - width;
   int rowDown = i + width;

   bool outOfBounds = (row <= 0) || (row > height) || (col <= 0) || (col > width);
   #else
   int rowUp = i - width;
   int rowDown = i + width;
   bool outOfBounds = (i < width);
   outOfBounds |= (i > (width * (height-1)));
   outOfBounds |= (i % width == 0);
   outOfBounds |= (i % width == width-1);
   #endif

   if (outOfBounds)
   {
	   output[i] = CELL_OFF;
	   return;
   }
   int neighbours = IsCellOn(input[rowUp-1]) + IsCellOn(input[rowUp]) + IsCellOn(input[rowUp+1]);
   neighbours += IsCellOn(input[i-1]) + IsCellOn(input[i+1]);
   neighbours += IsCellOn(input[rowDown-1]) + IsCellOn(input[rowDown]) + IsCellOn(input[rowDown+1]);
   if (neighbours == 3 || (IsCellOn(input[i]) && neighbours == 2))
       output[i] = CELL_ON;
   else
       output[i] = CELL_OFF;
}
