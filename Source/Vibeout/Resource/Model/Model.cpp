// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT
#include "PCH.h"
#include "Model.h"
#include "Vibeout/Resource/Material/Material.h"
#include "Vibeout/Resource/Manager/ResourceLoader.h"
#include "Vibeout/Resource/Texture/Texture.h"

namespace
{
    struct IndexHash
    {
        size_t operator()(const tinyobj::index_t& index) const
        {
            size_t h1 = std::hash<int>{}(index.vertex_index);
            size_t h2 = std::hash<int>{}(index.normal_index);
            size_t h3 = std::hash<int>{}(index.texcoord_index);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    struct IndexEqual
    {
        bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const
        {
            return a.vertex_index == b.vertex_index &&
                a.normal_index == b.normal_index &&
                a.texcoord_index == b.texcoord_index;
        }
    };

    using VertexMap = std::unordered_map<tinyobj::index_t, uint32, IndexHash, IndexEqual>;
}

Model::~Model()
{

}

bool Model::OnLoad(ResourceLoader& loader)
{
    VO_TRY(Base::OnLoad(loader));
    const std::filesystem::path path = loader.GetAssetPath();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> inputShapes;
    std::vector<tinyobj::material_t> inputMaterials;

    std::string warningMsg;
    std::string errorMsg;
    if (!tinyobj::LoadObj(&attrib, &inputShapes, &inputMaterials, &warningMsg, &errorMsg, path.string().c_str()))
    {
        VO_ERROR("Failed to load: {}: ", loader.GetId(), errorMsg);
        return false;
    }

    std::filesystem::path folder = path.parent_path();

    for (const tinyobj::material_t& inputMaterial : inputMaterials)
    {
        std::shared_ptr<Material> material(new Material());
        VO_TRY(material->OnLoad(loader, inputMaterial, folder.string()), "Failed to load a material of {}", loader.GetId());
        _materials.emplace_back(material);
    }
    
    VertexMap vertexMap;
   
    _shapes.resize(inputShapes.size());
    for (int i = 0; i < inputShapes.size(); ++i)
    {
        const auto& inputShape = inputShapes[i];
        Shape& shape = _shapes[i];
        const tinyobj::mesh_t& mesh = inputShape.mesh;
        shape._indices.resize(mesh.indices.size());
        for (int i = 0; i < mesh.indices.size(); ++i)
        {
            const tinyobj::index_t& index = mesh.indices[i];
            auto it = vertexMap.find(index);
            uint32 vertexIdx;
            if (it == vertexMap.end())
            {
                vertexIdx = (uint32)_vertices.size();
                vertexMap[index] = vertexIdx;
                Vertex& vertex = _vertices.emplace_back();
                if (index.vertex_index >= 0)
                {
                    vertex._pos[0] = attrib.vertices[3 * index.vertex_index + 0];
                    vertex._pos[1] = attrib.vertices[3 * index.vertex_index + 1];
                    vertex._pos[2] = attrib.vertices[3 * index.vertex_index + 2];
                }
                if (index.normal_index >= 0)
                {
                    vertex._normal[0] = attrib.normals[3 * index.normal_index + 0];
                    vertex._normal[1] = attrib.normals[3 * index.normal_index + 1];
                    vertex._normal[2] = attrib.normals[3 * index.normal_index + 2];
                }
                if (index.texcoord_index >= 0)
                {
                    vertex._uvs[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                    vertex._uvs[1] = attrib.texcoords[2 * index.texcoord_index + 1];
                }
            }
            else
            {
                vertexIdx = it->second;
            }
            shape._indices[i] = vertexIdx;
        }
    }
    return true;
}
