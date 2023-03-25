#ifndef LIFESCALAR_H_
#define LIFESCALAR_H_

#define REGULAR_COLORS

#ifdef REGULAR_COLORS
constexpr unsigned int CELL_ON = 0xFF000000;
constexpr unsigned int CELL_OFF = 0xFFFFFFFF;
constexpr unsigned int CELL_ON_RED = 0xFFFF0000;
constexpr unsigned int CELL_ON_BLUE = 0xFF0000FF;
#else
constexpr unsigned int CELL_ON = 0xFF582A72;
constexpr unsigned int CELL_OFF = 0xFF7B9F35;
constexpr unsigned int CELL_ON_RED = 0xFFAA3939;
constexpr unsigned int CELL_ON_BLUE = 0xFF0000FF;
#endif


void life(
   const int* input,
   int* output,
   int i,
   const int height,
   const int width, unsigned int oncolor);

#endif /* LIFESCALAR_H_ */
