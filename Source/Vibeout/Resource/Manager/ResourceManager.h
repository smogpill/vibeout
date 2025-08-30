// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Singleton.h"
#include "ResourceHolder.h"
#include "Vibeout/Resource/Resource.h"

class ResourceManager : public Singleton<ResourceManager>
{
public:
	ResourceManager(const std::string& assetsPath);
	~ResourceManager();
	
	template <class T, class F>
	auto LoadAsync(const std::string& id, F&& onDone) -> ResourceHandle<T>;
	auto GetAssetsPath() const -> const std::string& { return _assetsPath; }

private:
	friend class ResourceHolder;

	template <class T>
	auto GetOrCreateHolder(const std::string& id) -> TypedResourceHolder<T>*;
	void DestroyHolder(ResourceHolder& holder);
	bool Load(ResourceHolder& holder);

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

template <class T, class F>
ResourceHandle<T> ResourceManager::LoadAsync(const std::string& id, F&& onDone)
{
	ResourceHandle<T> handle(GetOrCreateHolder<T>(id));
	auto func = [handle, onDone]()
		{
			const bool result = ResourceManager::s_instance->Load(*handle._holder);
			onDone(result);
		};
	auto future = std::async(std::launch::async, func);
	return handle;
}
