// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once

class Texture
{
public:
	Texture(const std::string& id, bool& result);
	~Texture();

	auto GetBuffer() const -> const void* { return _buffer; }
	uint GetWidth() const { return _width; }
	uint GetHeigth() const { return _height; }
	uint GetNbComponents() const { return _nbComponents; }
	bool Is16Bits() const { return _16Bits; }

private:
	bool Init(const std::string& id);

	uint8* _buffer = nullptr;
	uint _width = 0;
	uint _height = 0;
	uint _nbComponents = 0;
	bool _16Bits = false;
};
