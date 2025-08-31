// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "ResourceHolder.h"
#include "ResourceManager.h"
template <class T> class ResourceHandle;

class ResourceLoader
{
public:
	ResourceLoader(ResourceHolder& holder);
	template <class T>
	auto AddDependency(const std::string& id) -> ResourceHandle<T>;
	auto GetId() const -> const std::string&;
	auto GetAssetPath() const -> std::string;

private:
	ResourceHolder& _holder;
};

template <class T>
auto ResourceLoader::AddDependency(const std::string& id) -> ResourceHandle<T>
{
	ResourceManager* manager = ResourceManager::s_instance;
	ResourceHandle<T> handle = manager->GetHandle<T>(id);
	handle._holder->_loadingParent = &_holder;
	_holder.AddLoadingDependency();
	handle._holder->LoadAsync();
	return handle;
}
