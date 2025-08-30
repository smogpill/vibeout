// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "ResourceManager.h"

ResourceHolder::ResourceHolder(const std::string& id)
	: _id(id)
{
}

void ResourceHolder::WaitReady()
{
	VO_ASSERT(false);
}

void ResourceHolder::OnAllRefsRemoved()
{
	VO_ASSERT(ResourceManager::s_instance);
	ResourceManager::s_instance->DestroyHolder(*this);
}

void ResourceManager::DestroyHolder(ResourceHolder& holder)
{
	_mutex.lock();
	auto it = _map.find(holder.GetId());
	_map.erase(it);
	_mutex.unlock();
	delete &holder;
}
