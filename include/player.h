#ifndef PLAYER
#define PLAYER

#include "entity.h"
#include "geometry.h"


typedef struct player
{
    XYZ where, velocity;
    float angle, anglesin, anglecos, yaw;
    unsigned sector;
    EntityState state;
} Player;

static Player player;

#endif
