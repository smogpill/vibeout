// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Texture.h"
#include "Vibeout/Resource/Manager/ResourceLoader.h"

Texture::~Texture()
{
	delete[] _buffer;
}

bool Texture::OnLoad(ResourceLoader& loader)
{
	VO_TRY(Base::OnLoad(loader));
	const std::string& path = loader.GetAssetPath();

	const int sixteenBits = stbi_is_16_bit(path.c_str()) != 0;

	int width, height, nbComponentsInFile;
	if (!stbi_info(path.c_str(), &width, &height, &nbComponentsInFile))
	{
		VO_ERROR("Failed to gather info from {}: {}", loader.GetId(), stbi_failure_reason());
		return false;
	}

	int wantedNbComponents = nbComponentsInFile;
	if (nbComponentsInFile == 3)
	{
		// Better compat with Vulkan. Though should be converted in the Renderer instead
		wantedNbComponents = STBI_rgb_alpha;
	}

	if (sixteenBits)
	{
		stbi_us* result = stbi_load_16(path.c_str(), &width, &height, &nbComponentsInFile, wantedNbComponents);
		if (!result)
		{
			VO_ERROR("Failed to load {}: {}", loader.GetId(), stbi_failure_reason());
			return false;
		}
		const uint64 size = width * height * wantedNbComponents * sizeof(uint16);
		_buffer = new uint8[size];
		memcpy(_buffer, result, size);
		stbi_image_free(result);
	}
	else
	{
		stbi_uc* result = stbi_load(path.c_str(), &width, &height, &nbComponentsInFile, wantedNbComponents);
		if (!result)
		{
			VO_ERROR("Failed to load {}: {}", loader.GetId(), stbi_failure_reason());
			return false;
		}
		const uint64 size = width * height * wantedNbComponents * sizeof(uint8);
		_buffer = new uint8[size];
		memcpy(_buffer, result, size);
		stbi_image_free(result);
	}

	_width = width;
	_height = height;
	_nbComponents = wantedNbComponents;
	_16Bits = sixteenBits != 0;
	return true;
}
