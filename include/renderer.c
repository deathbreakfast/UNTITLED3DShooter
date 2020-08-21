#include <SDL2/SDL.h>

#include "color.h"
#include "geometry.h"
#include "mathlib.h"
#include "constants.h"


static SDL_Renderer * renderer = NULL;

int lerp(int min, int max, int a, int b)
{
    return min + (max - min) * ((a - min) / (b - max));
}

// Linear interpolate textures
// Since we render each sector starting on the left edge with a horizontal edge we want to
// perform interpolation such that the 2d texture aligns itself with the orientation of the wall.
//   x, y = (0, 0)  x, y = ( , )
//   v              /
//   +----------+ <
//   |..........|
//   |..........|
//   |..........|
//   +----------+
//
//    -----------
//   /.../       \
//  /___/ < texture
// /               \
//
SDL_Color linearinterpolate(SDL_Surface * texture, int gx0, int gx1, int gy0, int gy1, int tx0, int tx1, int ty0, int ty1, int ipx, int ipy)
{
    // Texture width = 256
    // Geo width = 700
    // get the remainder at the end to account for tiling
    
    int ty = 0;
    int tx = 0;
    
    if (gy1 != gx0) {
        tx = ty0 + ((ty1 - ty0) * (ipy - gy0)) / (gy1 - gy0);
    }
    
    if (gx1 != gx0) {
        ty = tx0 + ((tx1 - tx0) * (ipx - gx0)) / (gx1 - gx0);
    }
    
    SDL_Color color;
    int bpp = texture->format->BytesPerPixel;
    
    Uint8 *data = (Uint8 *)texture->pixels + (tx % texture->w) * texture->pitch + (ty % texture->h) * bpp;
    color.r = data[0];
    color.g = data[1];
    color.b = data[2];
    
    return color;
}

void rendervline(int x, int y1, int y2, SDL_Color color, SDL_Color * texture)
{
    // Render each pixel starting from the top down.
    int col_num = 0;
    for (unsigned i = y1; i <= y2 || i < 0; i++)
    {
        // Set the color
        // If it is at the top or bottom point we want to render black to add a boarder.
        if (i == y1 || i == y2) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        }
        else {
            if (texture)
            {
                color = texture[col_num];
                col_num++;
            }
            
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
        }
        
        // Draw pixel
        SDL_RenderDrawPoint(renderer, x, i);
    }
}

SDL_Color get_color(int * color_num)
{
    if (*color_num > 4)
    {
        *color_num = 0;
    }
    switch (*color_num)
    {
        case 0: return charcoal;
        case 1: return pers_green;
        case 2: return orang_yellow;
        case 3: return sandy_brown;
        case 4: return burnt_sen;
    }
    return charcoal;
}

void drawscreen()
{
    // Use a rendering queue. As we find sectors that needs to render we will add them to the queue.
    enum { MaxQueue = 32 };
    Item queue[MaxQueue];
    Item * head = queue;
    Item * tail = queue;
    SDL_Color * color_col = malloc(sizeof(*color_col));
    size_t color_colbuffersize = sizeof(*color_col);
    
    // We want to set and store where the top and bottom boarders are for each section at each x cord.
    int ytop[ScreenWidth] = {0};
    int ybottom[ScreenWidth];
    
    for (unsigned x=0; x<ScreenWidth; ++x)
    {
        ybottom[x] = ScreenHeight - 1;
    }
    
    int renderedsectors[NumSectors];

    for (unsigned n=0; n<NumSectors; ++n)
    {
        renderedsectors[n] = 0;
    }

    // Begin whole-screen rendering using the sector where the player currently is.
    *head = (Item) { player.sector, 0, ScreenWidth-1 };
    if (++head == queue+MaxQueue)
    {
        head = queue;
    }

    do
    {
        // Pick a sector & slice from the queue to drawl
        const Item now = *tail;
        if (++tail == queue+MaxQueue)
        {
            tail = queue;
        }

        // Use bitwise operator to see if we need to give up.
        if (renderedsectors[now.sectorno] & 0x21)
        {
            continue;
        }
        
        ++renderedsectors[now.sectorno];
        const Sector * sect = &sectors[now.sectorno];
        int color_num = -1;
        for (unsigned s = 0; s < sect->npoints; s++)
        {
            color_num++;
            
            // Transform vertex relative to the player view.
            // This will move the room around the player.
            // P == player, -- == orientation, L = vertex, t = target location
            // ..........|.....
            // .......<-.L_____
            // ....--P...|.....
            // ..t.......V.....
            float vx1 = sect->vertex[s].x - player.where.x;
            float vx2 = sect->vertex[s+1].x - player.where.x;
            float vy1 = sect->vertex[s].y - player.where.y;
            float vy2 = sect->vertex[s+1].y - player.where.y;
                
            // Rotate the room to the correct orientation.
            // P == player, -- == orientation,  /  == vertex (at intersection)
            //                                  \
            // ....../..........
            // ...../...........
            // ./../--P.........
            // .\..\............
            // ..`>.\...........
            // Reference geometry of rotation:
            //   https://www.khanacademy.org/partner-content/pixar/sets/rotation/v/sets-8
            // We want to find t (target) tx and ty cordinates given the players current orientation.
            // P == player,  v == the vertex in the sector
            // )8 == the angle of the player (sin / cos relative to world space)
            //
            // We can get tx1 = vx1 * player cos - vy1 * player sin
            // and get tz1 = vx1 * player sin - vy1 * player cos
            //  |............................
            // ^|...t (tx1, tz1).............
            // ||../........v (vx1, vy1).....
            // y|./..........................
            // 0|P)8.........................
            //  L-----------------------------
            //   0 x -->
            float pcos = player.anglecos;
            float psin = player.anglesin;
            
            float tx1 = vx1 * psin - vy1 * pcos;
            float tz1 = vx1 * pcos + vy1 * psin;
            float tx2 = vx2 * psin - vy2 * pcos;
            float tz2 = vx2 * pcos + vy2 * psin;

            // Check if a wall is partially in front of the player.
            if (tz1 <= 0 && tz2 <= 0)
            {
                continue;
            }

            // If it is partially behind the player clip it
            if (tz1 <= 0 || tz2 <= 0)
            {
                float nearz = 1e-4f, farz = 5, nearside = 1e-5f, farside = 20.f;

                // Find the intersection between the player view and visable wall
                XY i1 = Intersect(tx1, tz1, tx2, tz2, -nearside, nearz, -farside, farz);
                XY i2 = Intersect(tx1, tz1, tx2, tz2, nearside, nearz, farside, farz);
                if (tz1 < nearz)
                {
                    if (i1.y > 0)
                    {
                        tx1 = i1.x;
                        tz1 = i1.y;
                    }
                    else
                    {
                        tx1 = i2.x;
                        tz1 = i2.y;
                    }
                }
                if (tz2 < nearz)
                {
                    if (i1.y > 0)
                    {
                        tx2 = i1.x;
                        tz2 = i1.y;
                    }
                    else
                    {
                        tx2 = i2.x;
                        tz2 = i2.y;
                    }
                }
            }

            // Perform the perspective transformation.
            // This will make sure the correct field of view is being used.
            // TOOD: Adjustible FOV
            float xscale1 = hfov / tz1;
            float yscale1 = vfov / tz1;
            float xscale2 = hfov / tz2;
            float yscale2 = vfov / tz2;

            int x1 = ScreenWidth / 2 - (int)(tx1 * xscale1);
            int x2 = ScreenWidth / 2 - (int)(tx2 * xscale2);
            
            if (x1 >= x2 || x2 < now.sx1 || x1 > now.sx2)
            {
                continue; // Only render if it's visible
            }
            
            int neighbor = sect->neighbors[s];

            float yceil = sect->ceil - player.where.z;
            float yfloor = sect->floor - player.where.z;
            
            
            // Get the floor and ceil and transform around player view.
            float nyceil = 0;
            float nyfloor = 0;
            
            // Is another sector showing through this portal?
            if (neighbor >= 0)
            {
                nyceil  = sectors[neighbor].ceil  - player.where.z;
                nyfloor = sectors[neighbor].floor - player.where.z;
            }
            
            // Project our ceiling & floor heights into screen coordinates (Y coordinate)
            #define Yaw(y,z) (y + z * player.yaw)
            
            int y1a = ScreenHeight / 2 - (int)(Yaw(yceil, tz1) * yscale1);
            int y1b = ScreenHeight / 2 - (int)(Yaw(yfloor, tz1) * yscale1);
            int y2a = ScreenHeight / 2 - (int)(Yaw(yceil, tz2) * yscale2);
            int y2b = ScreenHeight / 2 - (int)(Yaw(yfloor, tz2) * yscale2);
            
            // The same for the neighboring sector
            int ny1a = ScreenHeight / 2 - (int)(Yaw(nyceil, tz1) * yscale1);
            int ny1b = ScreenHeight / 2 - (int)(Yaw(nyfloor, tz1) * yscale1);
            int ny2a = ScreenHeight / 2 - (int)(Yaw(nyceil, tz2) * yscale2);
            int ny2b = ScreenHeight / 2 - (int)(Yaw(nyfloor, tz2) * yscale2);
            
            

            int beginx = max(x1, now.sx1);
            int endx = min(x2, now.sx2);
            
            // To add a temp astetic I'm coloring each wall based on the side. We alternate between five colors.
            SDL_Color wall_color;
            for (int x = beginx; x <= endx && x <= ScreenWidth; x++)
            {
                // Render the wall!
                
                // Calculate the Z coordinate for this point. (Only used for lighting.)
                int z = ((x - x1) * (tz2-tz1) / (x2-x1) + tz1) * 8;
                // Acquire the Y coordinates for our ceiling & floor for this X coordinate. Clamp them.
                int ya = (x - x1) * (y2a-y1a) / (x2-x1) + y1a;
                int yb = (x - x1) * (y2b-y1b) / (x2-x1) + y1b;
                
                
                int cya = clamp(ya, ytop[x], ybottom[x]); // top
                int cyb = clamp(yb, ytop[x], ybottom[x]); // bottom
                
                
                // Render ceiling: everything above this sector's ceiling height.
                rendervline(x, ytop[x], cya, ceil_color, NULL);
                // Render floor: everything below this sector's floor height.
                rendervline(x, cyb, ybottom[x], floor_color, NULL);
                
                size_t new_buffer_size = sizeof(*color_col) * (cyb - cya);
                if (new_buffer_size > color_colbuffersize) {
                    color_col = realloc(color_col, new_buffer_size);
                }
                for (int i = 0; i < cyb - cya; i++)
                {
                    // linearinterpolate(SDL_Surface * texture, int gx0, int gx1, int gy0, int gy1, int tx0, int tx1, int ty0, int ty1, int ipx, int ipy)
//                    printf("%f %f %d %d\n", x1, x2, cya, cyb);
                    color_col[i] = linearinterpolate(images[0], 0, x2 - x1, 0, yb - ya, 0, images[0]->w, 0, images[0]->h, x, i);
                }
                
                // Check to see if there is another sector behind an edge
                if (neighbor >= 0)
                {
                    // Same for _their_ floor and ceiling
                    int nya = (x - x1) * (ny2a-ny1a) / (x2-x1) + ny1a;
                    int cnya = clamp(nya, ytop[x], ybottom[x]);
                    int nyb = (x - x1) * (ny2b-ny1b) / (x2-x1) + ny1b;
                    int cnyb = clamp(nyb, ytop[x], ybottom[x]);
                    
                    // If our ceiling is higher than their ceiling, render upper wall
                    unsigned r1 = 0x010101 * (255-z), r2 = 0x040007 * (31-z/8);
                    rendervline(x, cya, cnya, wall_color, color_col); // Between our and their ceiling

                    ytop[x] = clamp(max(cya, cnya), ytop[x], ScreenHeight-1);   // Shrink the remaining window below these ceilings
                    // If our floor is lower than their floor, render bottom wall
                    rendervline(x, cnyb+1, cyb, wall_color, color_col); // Between their and our floor
                    ybottom[x] = clamp(min(cyb, cnyb), 0, ybottom[x]); // Shrink the remaining window above these floors
                }
                else
                {
                    if (x == beginx || x == endx)
                    {
                        wall_color = (SDL_Color){0, 0, 0, 255};
                    }
                    // Render the wall of the sector
                    rendervline(x, cya, cyb, wall_color, color_col);
                }
            }
            
            // Schedule the neighboring sector for rendering within the window formed by this wall.
            if (neighbor >= 0 && endx >= beginx && (head + MaxQueue + 1 - tail) % MaxQueue)
            {
                *head = (Item) { neighbor, beginx, endx };
                if(++head == queue+MaxQueue) head = queue;
            }
        }
        ++renderedsectors[now.sectorno];
    } while (head != tail);
    free(color_col);
    SDL_RenderPresent(renderer);
}
