// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Map.h"
#include "Vibeout/Resource/Manager/ResourceManager.h"
#include "Vibeout/Map/MapResource.h"

Map::Map(const char* name)
{
	_resource = ResourceManager::s_instance->GetHandle<MapResource>(std::string("Maps/") + name);
}

void Map::LoadAsync(std::function<void(bool)> onDone)
{
	_onDoneLoading = onDone;
	_resource.LoadAsync();
	_resource.AddCallback([this](bool result) { OnResourceLoaded(result); });
}

void Map::OnResourceLoaded(bool result)
{
	_onDoneLoading(result);
}
