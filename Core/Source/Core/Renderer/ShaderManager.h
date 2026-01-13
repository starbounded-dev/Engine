#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>
#include <cstdint>

namespace Core::Renderer
{
    // Shader program handle wrapper
    struct ShaderProgram
    {
        uint32_t Handle = 0;
        std::string Name;
        std::filesystem::path VertexPath;
        std::filesystem::path FragmentPath;
        std::filesystem::path ComputePath;
        bool IsCompute = false;
        std::filesystem::file_time_type LastModified;
    };

    // Callback for when a shader is reloaded
    using ShaderReloadCallback = std::function<void(const std::string& shaderName, uint32_t newHandle)>;

    // Shader manager for centralized shader management and hot reload
    class ShaderManager
    {
    public:
        static ShaderManager& Get();

        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        // Load and register a shader
        uint32_t LoadGraphicsShader(const std::string& name,
            const std::filesystem::path& vertexPath,
            const std::filesystem::path& fragmentPath);

        uint32_t LoadComputeShader(const std::string& name,
            const std::filesystem::path& computePath);

        // Get shader by name
        uint32_t GetShader(const std::string& name) const;
        bool HasShader(const std::string& name) const;

        // Reload a specific shader
        bool ReloadShader(const std::string& name);

        // Check all shaders for file changes and reload if necessary
        void CheckForChanges();

        // Enable/disable auto hot reload
        void SetHotReloadEnabled(bool enabled) { m_HotReloadEnabled = enabled; }
        bool IsHotReloadEnabled() const { return m_HotReloadEnabled; }

        // Register callback for shader reload events
        void RegisterReloadCallback(const std::string& shaderName, ShaderReloadCallback callback);
        void UnregisterReloadCallback(const std::string& shaderName);

        // Clear all shaders
        void Clear();

        // Get all shader names
        std::vector<std::string> GetShaderNames() const;

    private:
        ShaderManager() = default;

        bool CheckFileModified(ShaderProgram& program);
        void NotifyReloadCallbacks(const std::string& shaderName, uint32_t newHandle);

    private:
        std::unordered_map<std::string, ShaderProgram> m_Shaders;
        std::unordered_map<std::string, std::vector<ShaderReloadCallback>> m_ReloadCallbacks;
        bool m_HotReloadEnabled = true;
    };
}
