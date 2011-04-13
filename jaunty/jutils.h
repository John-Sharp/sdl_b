//various utility functions that are made use of at various points

#ifndef JUTILS_H
#define JUTILS_H

#include <math.h>

#define magnitude(a, b) sqrt(pow(a, 2) + pow(b, 2))
#define bankround(a) ((a) - floor(a)  >= 0.5 ? ceil(a) : floor(a))

/* returns a number that is  a number, 'a', converted into the nearest number
 * that is a whole power of 2 (rounding up) */ 
#define mkp2(a) (int)powf(2.0, ceilf(logf((float)a)/logf(2.0)))

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	enum { RMASK = 0xff000000, GMASK = 0x00ff0000,
        BMASK = 0x0000ff00,	AMASK = 0x000000ff};
#else
	enum { RMASK = 0x000000ff,	GMASK = 0x0000ff00,
        BMASK = 0x00ff0000,	AMASK = 0xff000000};
#endif


//sets the variables pointed to by 'm' and 'c' to the value of the 
//gradient and y intercept of the line described between the points
//(x1, y1) and (x2, y2)
void calc_m_c(double x1, double y1, double x2, double y2, double *m, double *c);


//tests if the lines (x11, y11)->(x12, y12) and (x21,y21)->(x22,y22)
//intersect. Returns 0 if they don't, 2 if they are parallel and 
//coincident, 1 if they cross. x and y are set to the point of intersection
int lines_intersect(double x11,double y11,double x12, double y12,
                    double x21, double y21, double x22, double y22,
                    double *x, double *y);

/* returns the angle between the vector described by (x, y) and the positive
 * x axis, if x = y = 0 returns 0 */
double x_pos_angle(double x, double y);


#endif
