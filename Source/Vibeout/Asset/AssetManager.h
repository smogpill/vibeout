// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Texture; class Model; class Asset;

class AssetManager
{
public:
	template <class T>
	T* GetAsset(const std::string& path);
	Texture* GetTexture(const std::string& path);
	Model* GetModel(const std::string& path);

private:
};

template <class T>
T* AssetManager::GetAsset(const std::string& path)
{
	T* asset = new T();
	if (asset->Load(path))
		return asset;
	return nullptr;
}
