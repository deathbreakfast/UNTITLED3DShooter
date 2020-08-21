#ifndef GEOMETRY
#define GEOMETRY


typedef struct item {
    int sectorno, sx1, sx2;
} Item;

typedef struct xy
{
    float x, y;
} XY;

typedef struct xyz {
    float x, y, z;
} XYZ;

typedef struct sector
{
    float floor, ceil;
    struct xy * vertex;
    signed char *neighbors; // Neighbor sectors
    unsigned npoints; // Num of verticies
} Sector;

static Sector * sectors = NULL;
static unsigned NumSectors = 0;

#endif
