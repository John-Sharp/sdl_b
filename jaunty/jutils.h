//various utility functions that are made use of at various points

#ifndef JUTILS_H
#define JUTILS_H

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

#endif
