// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "CraftPack.h"
#include "Vibeout/Resource/Manager/ResourceLoader.h"
#include "Vibeout/Resource/Resource.h"
#include "Vibeout/Resource/Model/Model.h"

bool CraftPack::OnLoad(ResourceLoader& loader)
{
	const char* modelIds[] =
	{
		"Big/Models/Feisar/feisar_prototype.obj"
	};
	for (const char* modelId : modelIds)
	{
		ResourceHandle<Model> handle = loader.AddDependency<Model>(modelId);
		_models.emplace_back(std::move(handle));
	}
	return true;
}
