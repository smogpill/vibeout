// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "ResourceManager.h"
#include "ResourceLoader.h"

ResourceManager::ResourceManager(const std::string& assetsPath)
	: _assetsPath(assetsPath)
{

}

ResourceManager::~ResourceManager()
{
	std::scoped_lock lock(_mutex);
	VO_ASSERT(_map.empty());
}

void ResourceManager::DestroyHolder(ResourceHolder& holder)
{
	_mutex.lock();
	auto it = _map.find(holder.GetId());
	_map.erase(it);
	_mutex.unlock();
}

ResourceHolder* ResourceManager::GetOrCreateHolder(const std::string& id)
{
	auto it = _map.find(id);
	if (it == _map.end())
	{
		ResourceHolder* holder = new ResourceHolder(id);
		_map[id] = holder;
		return holder;
	}
	else
	{
		return it->second;
	}
}
