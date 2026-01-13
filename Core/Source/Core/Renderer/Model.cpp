#include "Model.h"

#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Core::Renderer
{
    // -------- simple cache --------
    static std::unordered_map<std::string, std::weak_ptr<Model>> s_ModelCache;

    std::shared_ptr<Model> Model::LoadCached(const std::filesystem::path& path, bool flipUVs)
    {
        const std::string key = std::filesystem::weakly_canonical(path).string();

        if (auto it = s_ModelCache.find(key); it != s_ModelCache.end())
        {
            if (auto alive = it->second.lock())
                return alive;
        }

        auto model = std::make_shared<Model>(path, flipUVs);
        s_ModelCache[key] = model;
        return model;
    }

    Model::Model(const std::filesystem::path& path, bool flipUVs)
    {
        Load(path, flipUVs);
    }

    static std::string AssimpGetTexturePath(aiMaterial* mat, aiTextureType type)
    {
        aiString str;
        if (mat->GetTextureCount(type) > 0 && mat->GetTexture(type, 0, &str) == AI_SUCCESS)
            return str.C_Str();
        return {};
    }

    void Model::Load(const std::filesystem::path& path, bool flipUVs)
    {
        m_SourcePath = path;
        m_Directory = path.parent_path();

        Assimp::Importer importer;

        unsigned flags =
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality |
            aiProcess_SortByPType |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace;

        if (flipUVs)
            flags |= aiProcess_FlipUVs;

        const aiScene* scene = importer.ReadFile(path.string(), flags);
        if (!scene || !scene->mRootNode)
        {
            // replace with your logger
            // importer.GetErrorString()
            return;
        }

        // ---- materials ----
        m_Materials.clear();
        m_Materials.reserve(scene->mNumMaterials);

        for (unsigned i = 0; i < scene->mNumMaterials; i++)
        {
            aiMaterial* mat = scene->mMaterials[i];

            ModelMaterialInfo info;
            aiString name;
            if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
                info.Name = name.C_Str();

            info.AlbedoPath            = AssimpGetTexturePath(mat, aiTextureType_BASE_COLOR);
            if (info.AlbedoPath.empty())
                info.AlbedoPath        = AssimpGetTexturePath(mat, aiTextureType_DIFFUSE);

            info.NormalPath            = AssimpGetTexturePath(mat, aiTextureType_NORMALS);
            info.MetallicRoughnessPath = AssimpGetTexturePath(mat, aiTextureType_METALNESS);
            info.EmissivePath          = AssimpGetTexturePath(mat, aiTextureType_EMISSIVE);

            m_Materials.push_back(std::move(info));
        }

        // ---- meshes ----
        m_Meshes.clear();

        // recursive walk
        std::function<void(aiNode*)> processNode = [&](aiNode* node)
        {
            for (unsigned i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

                std::vector<MeshVertex> vertices;
                std::vector<uint32_t> indices;

                vertices.reserve(mesh->mNumVertices);

                for (unsigned v = 0; v < mesh->mNumVertices; v++)
                {
                    MeshVertex vert;
                    vert.Position = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };

                    if (mesh->HasNormals())
                        vert.Normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };

                    if (mesh->mTextureCoords[0])
                        vert.TexCoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };

                    if (mesh->HasTangentsAndBitangents())
                    {
                        vert.Tangent   = { mesh->mTangents[v].x,   mesh->mTangents[v].y,   mesh->mTangents[v].z };
                        vert.Bitangent = { mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z };
                    }

                    vertices.push_back(vert);
                }

                // indices (triangulated)
                for (unsigned f = 0; f < mesh->mNumFaces; f++)
                {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned j = 0; j < face.mNumIndices; j++)
                        indices.push_back(face.mIndices[j]);
                }

                uint32_t materialIndex = mesh->mMaterialIndex;
                m_Meshes.emplace_back(std::move(vertices), std::move(indices), materialIndex);
            }

            for (unsigned c = 0; c < node->mNumChildren; c++)
                processNode(node->mChildren[c]);
        };

        processNode(scene->mRootNode);
    }

    void Model::Draw() const
    {
        for (const auto& mesh : m_Meshes)
            mesh.Draw();
    }
}
