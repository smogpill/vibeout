// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "ResourceLoader.h"
#include "ResourceHolder.h"
#include "ResourceManager.h"

ResourceLoader::ResourceLoader(ResourceHolder& holder)
	: _holder(holder)
{
}

auto ResourceLoader::GetAssetPath() const -> std::string
{
	return (std::filesystem::path(ResourceManager::s_instance->GetAssetsPath()) / GetId()).string();
}

auto ResourceLoader::GetId() const -> const std::string&
{
	return _holder.GetId();
}
