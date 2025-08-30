// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Singleton.h"
#include "ResourceHolder.h"

class ResourceManager : public Singleton<ResourceManager>
{
public:
	ResourceManager(const std::string& assetsPath);
	~ResourceManager();

	template <class T>
	auto GetOrCreateHolder(const std::string& id) -> TypedResourceHolder<T>*;

	void DestroyHolder(ResourceHolder& holder);
	auto GetAssetPathFromId(const std::string& id) const -> std::string;

private:
	mutable std::mutex _mutex;
	std::unordered_map<std::string, ResourceHolder*> _map;
	std::string _assetsPath;
};

template <class T>
TypedResourceHolder<T>* ResourceManager::GetOrCreateHolder(const std::string& id)
{
	std::scoped_lock lock(_mutex);
	auto it = _map.find(id);
	if (it == _map.end())
	{
		TypedResourceHolder<T>* holder = new TypedResourceHolder<T>(id);
		_map[id] = holder;
		return holder;
	}
	else
	{
		return static_cast<TypedResourceHolder<T>*>(it->second);
	}
}
