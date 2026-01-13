#pragma once
#include <vector>
#include <string>
#include <memory>
#include <filesystem>

#include "Core/Renderer/Mesh.h"

namespace Core::Renderer
{
    struct ModelMaterialInfo
    {
        // Keep it simple for now (paths). Later you can turn these into Texture2D refs.
        std::string Name;
        std::string AlbedoPath;
        std::string NormalPath;
        std::string MetallicRoughnessPath;
        std::string EmissivePath;
    };

    class Model
    {
    public:
        static std::shared_ptr<Model> LoadCached(const std::filesystem::path& path, bool flipUVs = true);

        explicit Model(const std::filesystem::path& path, bool flipUVs = true);

        void Draw() const;

        const std::vector<Mesh>& GetMeshes() const { return m_Meshes; }
        const std::vector<ModelMaterialInfo>& GetMaterials() const { return m_Materials; }

    private:
        void Load(const std::filesystem::path& path, bool flipUVs);

    private:
        std::vector<Mesh> m_Meshes;
        std::vector<ModelMaterialInfo> m_Materials;

        std::filesystem::path m_Directory;
        std::filesystem::path m_SourcePath;
    };
}
