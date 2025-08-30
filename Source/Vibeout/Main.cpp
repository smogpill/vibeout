// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include <SDL3/SDL_main.h>
#include "Game/Game.h"
#include "Render/Renderer.h"
#include "Physics/PhysicsWorld.h"
#include "Vibeout/Resource/Manager/ResourceManager.h"

void ToggleFullscreen(SDL_Window* window)
{
    SDL_WindowFlags flags = SDL_GetWindowFlags(window);
    if (flags & SDL_WINDOW_FULLSCREEN)
    {
        SDL_SetWindowFullscreen(window, 0);
        //SDL_SetWindowSize(window, 1280, 720); // Reset to windowed size
    }
    else
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
}

int main(int argc, char* argv[])
{
    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        VO_ERROR("SDL could not initialize! SDL_Error: {}", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Vibeout", 1024, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!window)
    {
        VO_ERROR("Window could not be created! SDL_Error: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    {
        ResourceManager resourceManager;
        PhysicsWorld physicsWorld;
        Game game;
        game.SetWorld("Rugged");

        bool result;
        Renderer renderer(*window, game, result);
        if (!result)
        {
            VO_ERROR("Could not create the renderer");
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
            {
                switch (event.type)
                {
                case SDL_EVENT_QUIT: running = false; break;
                case SDL_EVENT_WINDOW_RESIZED:
                {
                    int width = 1024;
                    int height = 768;
                    SDL_GetWindowSizeInPixels(window, &width, &height);
                    renderer.OnWindowResized();
                    break;
                }
                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                {
                    SDL_SetWindowRelativeMouseMode(window, true);
                    break;
                }
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                {
                    SDL_SetWindowRelativeMouseMode(window, false);
                    break;
                }
                case SDL_EVENT_KEY_DOWN:
                {
                    const bool* states = SDL_GetKeyboardState(nullptr);
                    switch (event.key.key)
                    {
                    case SDLK_ESCAPE:
                    {
                        SDL_SetWindowRelativeMouseMode(window, false);
                        break;
                    }
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                    {
                        if (states[SDL_SCANCODE_LALT] || states[SDL_SCANCODE_RALT])
                            ToggleFullscreen(window);
                        break;
                    }
                    }
                       
                    break;
                }
                case SDL_EVENT_MOUSE_MOTION:
                {
                    if (SDL_GetWindowRelativeMouseMode(window))
                        game.OnMouseMotion(event.motion.xrel, event.motion.yrel);
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        SDL_SetWindowRelativeMouseMode(window, true);
                    break;
                }
            }

            game.Update(deltaTime);
            renderer.Render();
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
