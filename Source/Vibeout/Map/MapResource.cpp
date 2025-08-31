// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "MapResource.h"
#include "Vibeout/Resource/Manager/ResourceLoader.h"
#include "Vibeout/Resource/Texture/Texture.h"

bool MapResource::OnLoad(ResourceLoader& loader)
{
    VO_TRY(Base::OnLoad(loader));
    const std::string& id = loader.GetId();
    const std::string& assetPath = loader.GetAssetPath();
    VO_TRY(std::filesystem::exists(assetPath), "The map path does not exist: {}", assetPath);

    _heightmapTex = loader.AddDependency<Texture>(id + "/Heightmap.png");
    return true;
}
