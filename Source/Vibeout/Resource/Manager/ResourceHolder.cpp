// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "ResourceHolder.h"
#include "ResourceManager.h"

ResourceHolder::ResourceHolder(const std::string& id)
	: _id(id)
{
}

bool ResourceHolder::Load()
{
	return OnLoad();
}

void ResourceHolder::WaitReady()
{
	VO_ASSERT(false);
}

void ResourceHolder::OnAllRefsRemoved()
{
	VO_ASSERT(ResourceManager::s_instance);
	ResourceManager::s_instance->DestroyHolder(*this);
	Base::OnAllRefsRemoved();
}
