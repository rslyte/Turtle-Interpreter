#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "turtle.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

/*
 * Turtle's current state, which is pretty much the state of the pen, which include
 *  - location (x,y)
 *  - pen up or down state
 *  - pen direction
 */
struct Turtle {
  float x, y;   /* position */
  int penUp;    /* pen up? */
  float dir;    /* direction (0=>east, pi/2=>north) */
} turtle;

/*
 * Turtle state stack.
 */
#define MAX_TURTLE_STACK_SIZE 500 
static struct Turtle turtleStack[MAX_TURTLE_STACK_SIZE];
static struct Turtle *tsp = turtleStack;

/*
 * Black and white image holding turtle's drawings.
 */
#define IMAGE_SIZE 250    /* actual width and height of image */
unsigned char image[IMAGE_SIZE][IMAGE_SIZE];

/* initialization: 
 *  - the pen is at location (0.0, 0.0)
 *  - the pen is down by default
 *  - the pen direction is EAST */
void turtleInit(void) {
  turtle.x = turtle.y = 0.0;
  turtle.penUp = 0;
  turtle.dir = 0.0;
}

/*
 * Write the image content into a file in pgm format
 * */
void turtleDumpImage(FILE *f) {
  /* write pgm header information */
  fprintf(f, "P5\n");
  fprintf(f, "# turtle image\n");
  fprintf(f, "%d %d\n", IMAGE_SIZE, IMAGE_SIZE);
  fprintf(f, "255\n");

  /* write the actual image content */
  fwrite(image, 1, IMAGE_SIZE*IMAGE_SIZE, f);
}

/*
 * Move the pen to the initial position (0.0, 0.0)
 * */
void turtleHome(void) {
  turtle.x = turtle.y = 0.0;
}

/*
 * Set the pen up or down according to flag
 *  flag = 0: pen down
 *  flag = 1: pen up (can't draw)
 * */
void turtlePenUp(int flag) {
  turtle.penUp = flag;
}

/*
 * Push the curernt state of the turtle onto
 * the stack
 * */
void turtlePushState(void) {
  if (tsp - turtleStack >= MAX_TURTLE_STACK_SIZE) {
    fprintf(stderr, "Turtle stack overflow!\n");
    exit(-1);
  }
  *(tsp++) = turtle;
}

/*
 * pop the state from the stack
 * */

void turtlePopState(void) {
  if (tsp <= turtleStack) {
    fprintf(stderr, "Turtle stack underflow!\n");
    exit(-1);
  }
  turtle = *(--tsp);
}

/*
 * Map from turtle "world space" to image space.
 */
#define WORLD_SIZE 40.0   /* logical width and height of visible "world" */
/* Macros for converting x, y indices to actual pixel coordinates */
#define X2COL(x) ((IMAGE_SIZE/WORLD_SIZE)*(x) + IMAGE_SIZE/2)
#define Y2ROW(y) (IMAGE_SIZE/2 - (IMAGE_SIZE/WORLD_SIZE)*(y))

/*
 * Set the pixel at column c and row r to color white
 *
 * */
static void setPixel(int c, int r) {
  if (c >= 0 && c < IMAGE_SIZE &&
      r >= 0 && r < IMAGE_SIZE)
    // 255 is for white color
    image[r][c] = 255;
}

/*
 * Rasterize line given end points in "world coordinates."
 * Converts coords to image coords and uses simple DDA algo.
 */
static void drawLine(float x0, float y0, float x1, float y1) {
  float dx, dy;
  x0 = X2COL(x0); x1 = X2COL(x1);
  y0 = Y2ROW(y0); y1 = Y2ROW(y1);
  dx = x1-x0; dy = y1-y0;
  if (fabs(dx) >= fabs(dy)) {
    float dydx = dy/dx;
    int i,di,iend;
    if (dx > 0.0)
      di = 1;
    else {
      di = -1;
      dydx = -dydx;
    }
    i = (int) (x0 + 0.5);
    iend = (int) (x1 + 0.5);
    y0 += 0.5;
    while (i != iend) {
      setPixel(i,(int)y0);
      i += di;
      y0 += dydx;
    }
  } else {
    float dxdy = dx/dy;
    int j,dj,jend;
    if (dy > 0.0)
      dj = 1;
    else {
      dj = -1;
      dxdy = -dxdy;
    }
    j = (int) (y0 + 0.5);
    jend = (int) (y1 + 0.5);
    x0 += 0.5;
    while (j != jend) {
      setPixel((int)x0,j);
      j += dj;
      x0 += dxdy;
    }
  }
}

/*
 * implementation of FORWARD dist statement
 * draw a white line from current position and angle with length dist
 * */
void turtleForward(float dist) {
  float x0 = turtle.x, y0 = turtle.y;
  turtle.x += dist*cos(turtle.dir);
  turtle.y += dist*sin(turtle.dir);
  if (!turtle.penUp)
    drawLine(x0,y0, turtle.x,turtle.y);
}

/*
 * implementation of ANGLE ccwDegress statement
 * */
void turtleRotate(float ccwDegrees) {
#define DEG2RAD (M_PI/180)
  float phi = DEG2RAD*ccwDegrees;
  turtle.dir += phi;
}



