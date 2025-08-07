// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include <SDL3/SDL_main.h>
#include "Game/Game.h"
#include "Render/Renderer.h"

bool HandleEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_QUIT: return false;
    case SDL_EVENT_WINDOW_RESIZED: 
    {
        break;
    }
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    {
        break;
    }
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    {
        break;
    }
    }
    return true;
}

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

        uint64 lastTime = SDL_GetTicks();
        float deltaTime = 0.0f;

        bool running = true;
        SDL_Event event;
        while (running)
        {
            uint64 currentTime = SDL_GetTicks();
            deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            // Cap delta time to avoid spiral of death
            if (deltaTime > 0.05f)
                deltaTime = 0.05f;

            while (SDL_PollEvent(&event))
                if (!HandleEvent(event))
                    running = false;

            game.Update(deltaTime);
            renderer.Render();
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
