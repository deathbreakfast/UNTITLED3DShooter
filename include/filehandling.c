#include <SDL2/SDL_image.h>

#include "geometry.h"
#include "player.h"
#include "constants.h"


static SDL_Surface * images[256];
static int nimages = 0;

SDL_Color getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    SDL_LockSurface(surface);
    Uint32 p = *((Uint32 *)surface->pixels + y * surface->pitch + x * bpp);
    SDL_UnlockSurface(surface);
    Uint32 temp;
    SDL_PixelFormat *fmt = surface->format;
    Uint8 red, green, blue;

    temp = p & fmt->Rmask;  /* Isolate red component */
    temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
    temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
    red = (Uint8)temp;

    /* Get Green component */
    temp = p & fmt->Gmask;  /* Isolate green component */
    temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
    temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
    green = (Uint8)temp;

    /* Get Blue component */
    temp = p & fmt->Bmask;  /* Isolate blue component */
    temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
    temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
    blue = (Uint8)temp;
    
    printf("%d %d %d\n", red, green, blue);
    return (SDL_Color){red, green, blue, 255};
}


static void LoadData()
{
    FILE * fp = fopen(MapName, "rt");
    if (!fp)
    {
        perror(MapName);
        exit(1);
    }
    
    char Buf[256];
    char word[256];
    char * ptr;
    XY * vert = NULL;
    XY v;
    int n, m, NumVertices = 0;
    
    // Read a line the size of buffer and store it.
    while(fgets(Buf, sizeof Buf, fp))
    {
        // Grab up to 32 characters, store the word and check the first letter also take the length of the word
        //
        // Format:
        //     key used in switch
        //    /
        //   v
        // [0]
        //  v
        //  vertex    0    0 6 28
        //  |--------|
        //      \
        //       n = length
        switch(sscanf(ptr = Buf, "%32s%n", word, &n) == 1 ? word[0] : '\0')
        {
            case 'v': // Vertex
                // Iterate remaining current word buffer. Set x and y for current vertex then reallocate all verticies and store it.
                //
                // Format:
                //   y    x y  x
                //  /    / /  /
                // v    v v  v
                // 0    0 6 28
                // |---||-| |--|
                //   \   |   /
                //       n = remaining length
                for (sscanf(ptr += n, "%f%n", &v.y, &n); sscanf(ptr += n, "%f%n", &v.x, &n) == 1;)
                {
                    NumVertices++;
                    vert = realloc(vert, NumVertices * sizeof(*vert));
                    vert[NumVertices - 1] = v;
                }
                break;
            case 's': // Sector
                sectors = realloc(sectors, ++NumSectors * sizeof(*sectors));
                
                // Assign to local pointer so its easier to access
                Sector * sect = &sectors[NumSectors-1];
                int* num = NULL;
                
                // grab the floor and ceiling height
                //    floor
                //   / ceiling
                //  / /
                // v v
                // 0 20     3 14 29 49             -1 1 11 22
                sscanf(ptr += n, "%f%f%n", &sect->floor, &sect->ceil, &n);
                
                // For each remianing word in the string.
                // We want to make sure the word doesn't start with a '#' or we want to
                // stop there.
                // We also want to set -1 if an x is found.
                // TODO: Line comments will be # and x is used for no value.
                //   word
                //  /
                // v
                // 3 14 29 49             -1 1 11 22
                // |-|-||-|--------------|--|
                //  \ \  \       /        /
                //              n = remaining length
                for (m=0; sscanf(ptr += n, "%32s%n", word, &n) == 1 && word[0] != '#'; )
                {
                    num = realloc(num, ++m * sizeof(*num));
                    num[m-1] = word[0]=='x' ? -1 : atoi(word);
                }
                
                // Each vertice has a pair neighbor. There will be one additional vertex to account for
                // completing the loop.
                sect->npoints = m /= 2;
                sect->neighbors = malloc((m) * sizeof(*sect->neighbors));
                sect->vertex = malloc((m+1) * sizeof(*sect->vertex));
                
                // Paired vertice for the edge is stored mirrored in the string
                //                 Both sides contain n values
                //        /         |       \
                // |0, 1, ..n|            |0, 1, ..n|
                // 3 14 29 49             -1 1 11 22
                //        \                 /
                //        Verticies        Neighbor
                //
                //        [3, 14, 29, 49, -1, 1, 11, 22]
                //                        \
                //                        m = now equals halfway point
                for (n=0; n<m; ++n)
                {
                    sect->neighbors[n] = num[m + n];
                }
                for (n=0; n<m; ++n)
                {
                    sect->vertex[n+1]  = vert[num[n]]; // TODO: Range checking
                }
                sect->vertex[0] = sect->vertex[m]; // Ensure the vertexes form a loop
                free(num);
                break;
            case 'p':; // player
                float angle;
                // Only one line for player. Grab the x and y pos, the angle player is facing and sector
                sscanf(ptr += n, "%f %f %f %d", &v.x, &v.y, &angle, &n);
                player = (Player) {
                    {v.x, v.y, 0}, // z axis
                    {0,0,0}, // velocity
                    angle,
                    sinf(angle),
                    cosf(angle),
                    0, // yaw
                    n, // sector
                    {0, 0, 0, 0} // entity state
                }; // TODO: Range checking
                player.where.z = sectors[player.sector].floor + EyeHeight;
        }
    }
    fclose(fp);
    
    
    IMG_Init(IMG_INIT_PNG);
    SDL_Surface * img_test;
    images[0] = IMG_Load("resources/stonetiles_003_diff.png");
    nimages = 1;
    
    if(!images[0]) {
        printf("IMG_Load: %s\n", IMG_GetError());
        // handle error
    }
    free(vert);
}
