#include "input.h"

//TODO: JOYPAD Interrupt.

void HandleInput(SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {

        switch(event->key.keysym.sym) {
            case SDLK_d:      keys.A      = 0; break;
            case SDLK_a:      keys.B      = 0; break;
            case SDLK_LSHIFT: keys.Select = 0; break;
            case SDLK_SPACE:  keys.Start  = 0; break;

            case SDLK_UP:     keys.Up     = 0; break;
            case SDLK_DOWN:   keys.Down   = 0; break;
            case SDLK_LEFT:   keys.Left   = 0; break;
            case SDLK_RIGHT:  keys.Right  = 0; break;

            case SDLK_BACKSPACE: PrintRegisters(); break;
            case SDLK_TAB: waitForVSync = 0;       break;

            case SDLK_PAGEUP: ResizeWindow(1); break;
            case SDLK_PAGEDOWN: ResizeWindow(-1); break;
        }

    }
    else if (event->type == SDL_KEYUP) {

        switch(event->key.keysym.sym) {
            case SDLK_d:      keys.A      = 1; break;
            case SDLK_a:      keys.B      = 1; break;
            case SDLK_LSHIFT: keys.Select = 1; break;
            case SDLK_SPACE:  keys.Start  = 1; break;

            case SDLK_UP:     keys.Up     = 1; break;
            case SDLK_DOWN:   keys.Down   = 1; break;
            case SDLK_LEFT:   keys.Left   = 1; break;
            case SDLK_RIGHT:  keys.Right  = 1; break;

            case SDLK_TAB: waitForVSync = 1;   break;
        }

    }
}