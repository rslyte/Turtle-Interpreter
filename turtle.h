#ifndef TURTLE_H
#define TURTLE_H

#include <stdlib.h>
#include <stdio.h> //RYAN ADDED THIS IN 2/10

/* header file for turtle.c. It provides all interfaces for 
 * actual implementation of following functions in turtle.c */

/* initialize turtle state (state of the pen) 
 * At initialization
 *  - pen is down
 *  - pen is at location (0.0, 0.0)
 *  - pen direction is east */
void turtleInit(void);

/* write image content to a file in pgm format*/
void turtleDumpImage(FILE *f);

/* set the pen to the initial position (0.0, 0.0) */
void turtleHome(void);

/* set pen up or down based on flag
 *  - flag = 0: pen is down 
 *  - flag = 1: pen is up (will not draw)  */
void turtlePenUp(int flag);

/* move pen forward for a distance dist */
void turtleForward(float dist);

/* change pen direction to ccwDegrees relatively to its current direction */
void turtleRotate(float ccwDegrees);

void turtlePushState(void);
void turtlePopState(void);

#endif /* TURTLE_H */
