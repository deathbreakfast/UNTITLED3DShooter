//
//  main.c
//  UNTITLED3Dgame
//
//  Created by Sean O'Rourke on 6/30/20.
//  Copyright © 2020 Sean O'Rourke. All rights reserved.
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

#include "include/constants.h"
#include "include/filehandling.h"
#include "include/handleinput.h"
#include "include/geometry.h"
#include "include/player.h"
#include "include/playermovement.h"
#include "include/renderer.h"


static void UnloadData()
{
    // Clear the geometry
    for (unsigned i = 0; i < NumSectors; i++)
    {
        free(sectors[i].vertex);
    }
    for (unsigned i=0; i < NumSectors; i++) {
        free(sectors[i].neighbors);
    }
    free(sectors);
    sectors = NULL;
    
    // Clear the texture memory
    NumSectors = 0;
    for (unsigned i = 0; i < nimages; i++)
    {
       SDL_FreeSurface(images[i]);
    }
//    free(images);
}

void mainloop()
{
    int * wasd;
    wasd = malloc(sizeof(int) * 4);
    wasd[0] = 0;
    wasd[1] = 0;
    wasd[2] = 0;
    wasd[3] = 0;
    
    SDL_bool done = SDL_FALSE;
    while (!done)
    {
        SDL_Event event;
        
        drawscreen();
        collisiondetection();
        handleinput(&event, &done, wasd);
        handlemovement(wasd);
    }
}

int main(int argc, const char * argv[])
{
    LoadData();
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_Window* window = NULL;
        if (SDL_CreateWindowAndRenderer(ScreenWidth, ScreenHeight, 0, &window, &renderer) == 0) {
            mainloop();
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
    }
    UnloadData();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
