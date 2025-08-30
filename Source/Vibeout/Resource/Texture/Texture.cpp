// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Texture.h"
#include "Vibeout/Resource/Manager/ResourceManager.h"

Texture::Texture(const std::string& id, bool& result)
{
	result = Init(id);
}

Texture::~Texture()
{
	delete[] _buffer;
}

bool Texture::Init(const std::string& id)
{
	const std::string path = ResourceManager::s_instance->GetAssetPathFromId(id);

	const int sixteenBits = stbi_is_16_bit(path.c_str()) != 0;

	int width, height, nbComponentsInFile;
	if (!stbi_info(path.c_str(), &width, &height, &nbComponentsInFile))
	{
		VO_ERROR("Failed to gather info from {}: {}", id, stbi_failure_reason());
		return false;
	}

	if (sixteenBits)
	{
		stbi_us* result = stbi_load_16(path.c_str(), &width, &height, &nbComponentsInFile, nbComponentsInFile);
		if (!result)
		{
			VO_ERROR("Failed to load {}: {}", id, stbi_failure_reason());
			return false;
		}
		const uint64 size = width * height * nbComponentsInFile * sizeof(uint16);
		_buffer = new uint8[size];
		memcpy(_buffer, result, size);
		stbi_image_free(result);
	}
	else
	{
		stbi_uc* result = stbi_load(path.c_str(), &width, &height, &nbComponentsInFile, nbComponentsInFile);
		if (!result)
		{
			VO_ERROR("Failed to load {}: {}", id, stbi_failure_reason());
			return false;
		}
		const uint64 size = width * height * nbComponentsInFile * sizeof(uint8);
		_buffer = new uint8[size];
		memcpy(_buffer, result, size);
		stbi_image_free(result);
	}

	_width = width;
	_height = height;
	_nbComponents = nbComponentsInFile;
	_16Bits = sixteenBits != 0;
	return true;
}
