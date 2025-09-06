// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class Terrain;
class MapResource;

class Map
{
public:
	Map(const std::string& name);
	~Map();

	void LoadAsync(std::function<void(bool)> onDone);
	auto GetTerrain() const -> Terrain* { return _terrain; }

private:
	void OnResourceLoaded(bool result);
	bool InitTerrain();

	ResourceHandle<MapResource> _resource;
	Terrain* _terrain = nullptr;
	std::function<void(bool)> _onDoneLoading;
};
