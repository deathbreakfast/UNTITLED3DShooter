#include "geometry.h"
#include "player.h"
#include "mathlib.h"


/**
 * MovePlayer(dx,dy): Moves the player by (dx,dy) in the map, and
 * also updates their anglesin/anglecos/sector properties properly.
 */
static void MovePlayer(float dx, float dy)
{
    float px = player.where.x, py = player.where.y;
    
    // Check if this movement crosses one of this sector's edges
    // that have a neighboring sector on the other side.
    // Because the edge vertices of each sector are defined in
    // clockwise order, PointSide will always return -1 for a point
    // that is outside the sector and 0 or 1 for a point that is inside.
    const Sector * const sect = &sectors[player.sector];
    const struct xy* const vert = sect->vertex;
    for (unsigned s = 0; s < sect->npoints; ++s)
    {
        if
        (
            sect->neighbors[s] >= 0
            // The two 2D boxes would intercect, calculate from.
            && IntersectBox(px,py, px+dx,py+dy, vert[s].x, vert[s].y, vert[s+1].x, vert[s+1].y)
            // Make sure we are checking the correct side.
            && PointSide(px+dx, py+dy, vert[s].x, vert[s].y, vert[s+1].x, vert[s+1].y) < 0
        )
        {
            player.sector = sect->neighbors[s];
            break;
        }
    }

    // Move player
    player.where.x += dx;
    player.where.y += dy;
    
    player.anglesin = sinf(player.angle);
    player.anglecos = cosf(player.angle);
}

static void handlemovement(int wasd[4])
{
    
    // mouse aiming
    int x, y;
    float yaw = 0;
    SDL_GetRelativeMouseState(&x,&y);
    player.angle += x * 0.03f;
    yaw = clamp(yaw - y*0.05f, -5, 5);
    player.yaw = yaw - player.velocity.z*0.5f;
    MovePlayer(0, 0); // Currently calculating twice, maybe only need one?
    
    // We are moving relative to the viewport so we want to use a vector
    // to calculate the velocity.
    float movev[2] = {0.f, 0.f};
    
    // Move forward
    if (wasd[0])
    {
        movev[0] += player.anglecos*0.2f;
        movev[1] += player.anglesin*0.2f;
    }
    // Move left
    if (wasd[1])
    {
        movev[0] -= player.anglecos*0.2f;
        movev[1] -= player.anglesin*0.2f;
    }
    // Move back
    if (wasd[2])
    {
        movev[0] += player.anglesin*0.2f;
        movev[1] -= player.anglecos*0.2f;
    }
    // move foorward
    if (wasd[3])
    {
        movev[0] -= player.anglesin*0.2f;
        movev[1] += player.anglecos*0.2f;
    }
    
    int pushing = wasd[0] || wasd[1] || wasd[2] || wasd[3];
    
    // To keep momentum we want to start reducing speed instead of an abrupt stop.
    float acceleration = pushing ? 0.4 : 0.2;

    player.velocity.x = player.velocity.x * (1-acceleration) + movev[0] * acceleration;
    player.velocity.y = player.velocity.y * (1-acceleration) + movev[1] * acceleration;

    if (pushing)
    {
        player.state.moving = 1;
    }
}

static void collisiondetection()
{
    
    float eyeheight = player.state.ducking ? DuckHeight : EyeHeight;
    player.state.ground = !player.state.falling;
    
    if (player.state.falling)
    {
        // Add gravity
        player.velocity.z -= 0.05f;
        float nextz = player.where.z + player.velocity.z;
        
        // When going down
        if (player.velocity.z < 0 && nextz < sectors[player.sector].floor + eyeheight)
        {
            // Fix to ground
            player.where.z = sectors[player.sector].floor + eyeheight;
            player.velocity.z = 0;
            player.state.falling = 0;
            player.state.ground = 1;
        }
        
        // When going up
        else if (player.velocity.z > 0 && nextz > sectors[player.sector].ceil)
        {
            // Prevent jumping above ceiling
            player.velocity.z = 0;
            player.state.falling = 1;
        }
        if (player.state.falling)
        {
            player.where.z += player.velocity.z;
            player.state.moving = 1;
        }
    }
    
    // Horizontal collision detection
    if (player.state.moving)
    {
        float px = player.where.x;
        float py = player.where.y;
        float dx = player.velocity.x;
        float dy = player.velocity.y;

        const Sector* const sect = &sectors[player.sector];
        const struct xy* const vert = sect->vertex;
        
        // Check if the player is about to cross one of the sector's edges
        for (unsigned s = 0; s < sect->npoints; ++s)
            if
            (
                IntersectBox(px,py, px+dx,py+dy, vert[s+0].x, vert[s+0].y, vert[s+1].x, vert[s+1].y)
                && PointSide(px+dx, py+dy, vert[s+0].x, vert[s+0].y, vert[s+1].x, vert[s+1].y) < 0
            )
            {
                // Check height and floor of hole into another sector
                float hole_low = sect->neighbors[s] < 0 ?  9e9 : max(sect->floor, sectors[sect->neighbors[s]].floor);
                float hole_high = sect->neighbors[s] < 0 ? -9e9 : min(sect->ceil, sectors[sect->neighbors[s]].ceil);
                
                // Check whether we're bumping into a wall.
                if (hole_high < player.where.z+HeadMargin || hole_low  > player.where.z-eyeheight+KneeHeight)
                {
                    // Bumps into a wall! Slide along the wall.
                    // This formula is from Wikipedia article "vector projection".
                    float xd = vert[s+1].x - vert[s+0].x, yd = vert[s+1].y - vert[s+0].y;
                    dx = xd * (dx*xd + yd*dy) / (xd*xd + yd*yd);
                    dy = yd * (dx*xd + yd*dy) / (xd*xd + yd*yd);
                    player.state.moving = 0;
                }
            }
        MovePlayer(dx, dy);
        player.state.falling = 1;
    }
}
