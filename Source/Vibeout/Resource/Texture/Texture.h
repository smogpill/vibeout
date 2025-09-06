// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
#include "Vibeout/Resource/Resource.h"
class ResourceLoader;

class Texture : public Resource
{
	using Base = Resource;
public:
	~Texture();

	bool OnLoad(ResourceLoader& loader) override;

	auto GetBuffer() const -> const void* { return _buffer; }
	auto GetBufferSize8() const -> uint32 { return _width * _height * _nbComponents * (_16Bits ? 2 : 1); }
	uint GetWidth() const { return _width; }
	uint GetHeigth() const { return _height; }
	uint GetNbComponents() const { return _nbComponents; }
	bool Is16Bits() const { return _16Bits; }
	uint GetStrideBetweenTexels() const { return _nbComponents * (_16Bits ? 2 : 1); }

private:
	uint8* _buffer = nullptr;
	uint _width = 0;
	uint _height = 0;
	uint _nbComponents = 0;
	bool _16Bits = false;
};
