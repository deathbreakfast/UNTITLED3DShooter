#include <SDL2/SDL.h>

#include "player.h"


static int * handleinput(SDL_Event * event, SDL_bool * done, int * wasd)
{
    while (SDL_PollEvent(event))
    {
        switch(event->type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch(event->key.keysym.sym)
                {
                    case 'w':
                        wasd[0] = event->type==SDL_KEYDOWN;
                        break;
                    case 's':
                        wasd[1] = event->type==SDL_KEYDOWN;
                        break;
                    case 'a':
                        wasd[2] = event->type==SDL_KEYDOWN;
                        break;
                    case 'd':
                        wasd[3] = event->type==SDL_KEYDOWN;
                        break;
                    case 'q':
                        *done = SDL_TRUE;
                        break;
                    case ' ': /* jump */
                        if (player.state.ground)
                        {
                            player.velocity.z += 0.5; player.state.falling = 1;
                        }
                        break;
                    case SDLK_LCTRL: /* duck */
                    case SDLK_RCTRL:
                        player.state.ducking = event->type==SDL_KEYDOWN; player.state.falling=1;
                        break;
                    default: break;
                }
                break;
            case SDL_QUIT:
                *done = SDL_TRUE;
                break;
        }
    }
    return wasd;
}

