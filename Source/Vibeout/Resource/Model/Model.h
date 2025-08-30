// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#pragma once
class Material;

class Model
{
public:
	Model(const std::string& id, bool& result);
	
private:
	bool Init(const std::string& id);

	struct Vertex
	{
		float _pos[3];
		float _normal[3];
		float _uvs[2];
	};
	struct Shape
	{
		std::vector<uint32> _indices;
	};

	std::vector<Vertex> _vertices;
	std::vector<Shape> _shapes;
	std::vector<std::shared_ptr<Material>> _materials;
};
