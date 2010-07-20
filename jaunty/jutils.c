//various utility functions that are made use of at various points
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b) 

#define inside_box(x, y, vx1, vy1, vx2, vy2) \
            ( (y >= min(vy1, vy2) - 0.001\
            && x >= min(vx1, vx2) - 0.001\
            && y <= max(vy1, vy2) + 0.001\
            && x <= max(vx1, vx2) + 0.001) ? 1 : 0 )



void calc_m_c(double x1, double y1, double x2, double y2, double *m, double *c)
{
    /*calculate the denominator of the gradient of the line between
     * the two points, take this opportunity to swap (x1, y1) and
     * (x2, y2) to ensure that (x1, y1) is the point furthest on the left*/
    *m = x2 - x1;
    if(*m < 0){
        double s;
        s = x1;
        x1 = x2;
        x2 = s;

        s = y1;
        y1 = y2;
        y2 = s;
        *m *= -1;
    }

    /*finish calculating the gradient*/
    *m = (y2 - y1) / *m;

 
    /*calculate the y-intercept*/
    *c = (x2 * y1 - y2 * x1)/
         (x2 - x1);
    return;
}


int lines_intersect(double x11,double y11,double x12, double y12,
                    double x21, double y21, double x22, double y22,
                    double *x, double *y)
{
    double m[2], c[2];
    double sparex, sparey;

    if(x == NULL)
        x = &sparex;
    if(y == NULL)
        y = &sparey;

    calc_m_c(x11, y11, x12, y12, &m[0], &c[0]);
    calc_m_c(x21, y21, x22, y22, &m[1], &c[1]);

    if((m[0] == HUGE_VAL || -m[0] == HUGE_VAL) &&
            (m[1] == HUGE_VAL || -m[1] == HUGE_VAL)){
        if( x11 == x21 )
            return 2;
        else
            return 0;
    }else if(m[0] == HUGE_VAL || -m[0] == HUGE_VAL ){
        *x = x11;
        *y = m[1] * (*x) + c[1];
    }else if(m[1] == HUGE_VAL || -m[1] == HUGE_VAL){
        *x = x21;
        *y = m[0] * (*x) + c[0];
    }else if(m[1] == m[0]){
        if(c[0] == c[1])
            return 2;
        else
            return 0;
    }
    else{
        *x = (c[0] - c[1])/(m[1] - m[0]);
        *y = m[0]*(*x) + c[0];
    }

    if(inside_box(*x, *y, x11, y11, x12, y12) &&
            inside_box(*x, *y, x21, y21, x22, y22)){
        return 1;
    }


    return 0;
}


