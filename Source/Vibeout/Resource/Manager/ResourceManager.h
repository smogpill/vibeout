// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Base/Singleton.h"
#include "Vibeout/Base/Job/JobSystem.h"
#include "Vibeout/Resource/Resource.h"
#include "ResourceHolder.h"

class ResourceManager : public Singleton<ResourceManager>
{
public:
	ResourceManager(const std::string& assetsPath);
	~ResourceManager();
	
	template <class T>
	auto GetHandle(const std::string& id) -> ResourceHandle<T>;
	auto GetAssetsPath() const -> const std::string& { return _assetsPath; }

private:
	friend class ResourceHolder;

	auto GetOrCreateHolder(const std::string& id) -> ResourceHolder*;
	void DestroyHolder(ResourceHolder& holder);

	mutable std::mutex _mutex;
	std::unordered_map<std::string, ResourceHolder*> _map;
	std::string _assetsPath;
};

template <class T>
auto ResourceManager::GetHandle(const std::string& id) -> ResourceHandle<T>
{
	std::scoped_lock lock(_mutex);
	return ResourceHandle<T>(GetOrCreateHolder(id));
}
