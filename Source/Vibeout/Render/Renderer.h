// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class Renderer
{
public:
	Renderer(SDL_Window& window, bool& result);
	~Renderer();

private:
	bool Init();

	SDL_Window& _window;
	VkInstance _instance = nullptr;
	VkSurfaceKHR _surface = nullptr;
};
