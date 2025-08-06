// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include <SDL3/SDL_main.h>
#include "Game/Game.h"
#include "Render/Renderer.h"

int main(int argc, char* argv[])
{
    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Vibeout", 1024, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    {
        Game game;

        bool result;
        Renderer renderer(*window, game, result);
        if (!result)
        {
            printf("Could not create the renderer");
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        int quit = 0;
        SDL_Event e;
        while (!quit)
        {
            while (SDL_PollEvent(&e))
                if (e.type == SDL_EVENT_QUIT)
                    quit = 1;

            game.Update();
            renderer.Render();
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
