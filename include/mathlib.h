#ifndef MATHLIB
#define MATHLIB


/**
 *Utility functions
 * we use the slightly less safe preprocessor macros to implement these functions that work with multiple types. and we avoid adding function into call stack.
 */

// min: Choose smaller of two scalars.
#define min(a, b)             (((a) < (b)) ? (a) : (b))

// max: Choose greater of two scalars.
#define max(a, b)             (((a) > (b)) ? (a) : (b))

// clamp: Clamp value into set range.
#define clamp(a, mi, ma)      min(max(a,mi),ma)

// vxs: Vector cross product
// This is used to find the orthoganal vector
// [x0] * [y0]  =  [x0 * y1 - x1 * y0]
// [x1]   [y1]
#define vxs(x0, y0, x1, y1)    ((x0)*(y1) - (x1)*(y0))

// Overlap:  Determine whether the two number ranges overlap.
//  0 - 100 & 90 - 00
// Take lower value between 0 - 100 and it must be below the max value of 90 - 100
// Take the min value between 90 - 100 and it must be below the max of 0 - 100
// Return the result of both being true
#define Overlap(a0, a1, b0, b1) \
( \
    min(a0,a1) <= max(b0,b1) && min(b0,b1) <= max(a0,a1) \
)

// IntersectBox: Determine whether two 2D-boxes intersect.
#define IntersectBox(x0,y0, x1,y1, x2,y2, x3,y3) \
( \
    Overlap(x0,x1,x2,x3) && Overlap(y0,y1,y2,y3) \
)

// PointSide: Determine which side of a line the point is on. Return value: <0, =0 or >0.
#define PointSide(px,py, x0,y0, x1,y1) \
( \
    vxs((x1)-(x0), (y1)-(y0), (px)-(x0), (py)-(y0)) \
)

// Intersect: Calculate the point of intersection between two lines.
#define Intersect(x1,y1, x2,y2, x3,y3, x4,y4) \
( \
    (XY)\
    { \
        vxs(vxs(x1,y1, x2,y2), (x1)-(x2), vxs(x3,y3, x4,y4), (x3)-(x4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)), \
        vxs(vxs(x1,y1, x2,y2), (y1)-(y2), vxs(x3,y3, x4,y4), (y3)-(y4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)) \
    } \
)

#endif
