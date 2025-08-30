// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class ResourceLoader; class Model;

class CraftPack
{
public:
	bool OnLoad(ResourceLoader& loader);

private:
	std::vector<ResourceHandle<Model>> _models;
};
