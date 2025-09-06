// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Map.h"
#include "Vibeout/Resource/Manager/ResourceManager.h"
#include "Vibeout/Game/Map/MapResource.h"
#include "Vibeout/World/Terrain/Terrain.h"

Map::Map(const std::string& name)
{
	_resource = ResourceManager::s_instance->GetHandle<MapResource>(std::string("Maps/") + name);
}

Map::~Map()
{
	delete _terrain;
}

void Map::LoadAsync(std::function<void(bool)> onDone)
{
	_onDoneLoading = onDone;
	_resource.LoadAsync();
	_resource.AddCallback([this](bool result) { OnResourceLoaded(result); });
}

void Map::OnResourceLoaded(bool result)
{
	if (result)
	{
		result &= InitTerrain();
	}
	_onDoneLoading(result);
}

bool Map::InitTerrain()
{
	delete _terrain; _terrain = nullptr;
	const MapResource* mapResource = _resource.Get();
	VO_ASSERT(mapResource);
	bool result;
	_terrain = new Terrain(mapResource->GetHeightmapTex(), mapResource->GetDiffuseTex(), result);
	VO_TRY(result);
	return true;
}
