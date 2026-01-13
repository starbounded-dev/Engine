#include "ShaderManager.h"
#include "Shader.h"
#include <iostream>

namespace Core::Renderer
{
    ShaderManager& ShaderManager::Get()
    {
        static ShaderManager instance;
        return instance;
    }

    uint32_t ShaderManager::LoadGraphicsShader(const std::string& name,
        const std::filesystem::path& vertexPath,
        const std::filesystem::path& fragmentPath)
    {
        // Check if shader already exists
        auto it = m_Shaders.find(name);
        if (it != m_Shaders.end())
        {
            std::cerr << "Shader '" << name << "' already exists. Reloading...\n";
            ReloadShader(name);
            return it->second.Handle;
        }

        // Load the shader
        uint32_t handle = CreateGraphicsShader(vertexPath, fragmentPath);
        if (handle == static_cast<uint32_t>(-1))
        {
            std::cerr << "Failed to load shader: " << name << "\n";
            return static_cast<uint32_t>(-1);
        }

        // Register the shader
        ShaderProgram program;
        program.Handle = handle;
        program.Name = name;
        program.VertexPath = vertexPath;
        program.FragmentPath = fragmentPath;
        program.IsCompute = false;

        // Get last modified time
        try
        {
            auto vertTime = std::filesystem::last_write_time(vertexPath);
            auto fragTime = std::filesystem::last_write_time(fragmentPath);
            program.LastModified = std::max(vertTime, fragTime);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to get file modification time: " << e.what() << "\n";
        }

        m_Shaders[name] = program;
        return handle;
    }

    uint32_t ShaderManager::LoadComputeShader(const std::string& name,
        const std::filesystem::path& computePath)
    {
        // Check if shader already exists
        auto it = m_Shaders.find(name);
        if (it != m_Shaders.end())
        {
            std::cerr << "Shader '" << name << "' already exists. Reloading...\n";
            ReloadShader(name);
            return it->second.Handle;
        }

        // Load the shader
        uint32_t handle = CreateComputeShader(computePath);
        if (handle == static_cast<uint32_t>(-1))
        {
            std::cerr << "Failed to load compute shader: " << name << "\n";
            return static_cast<uint32_t>(-1);
        }

        // Register the shader
        ShaderProgram program;
        program.Handle = handle;
        program.Name = name;
        program.ComputePath = computePath;
        program.IsCompute = true;

        // Get last modified time
        try
        {
            program.LastModified = std::filesystem::last_write_time(computePath);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to get file modification time: " << e.what() << "\n";
        }

        m_Shaders[name] = program;
        return handle;
    }

    uint32_t ShaderManager::GetShader(const std::string& name) const
    {
        auto it = m_Shaders.find(name);
        if (it != m_Shaders.end())
        {
            return it->second.Handle;
        }
        return static_cast<uint32_t>(-1);
    }

    bool ShaderManager::HasShader(const std::string& name) const
    {
        return m_Shaders.find(name) != m_Shaders.end();
    }

    bool ShaderManager::ReloadShader(const std::string& name)
    {
        auto it = m_Shaders.find(name);
        if (it == m_Shaders.end())
        {
            std::cerr << "Shader '" << name << "' not found for reload.\n";
            return false;
        }

        ShaderProgram& program = it->second;
        uint32_t newHandle;

        if (program.IsCompute)
        {
            newHandle = ReloadComputeShader(program.Handle, program.ComputePath);
        }
        else
        {
            newHandle = ReloadGraphicsShader(program.Handle, program.VertexPath, program.FragmentPath);
        }

        // Check if reload was successful
        if (newHandle == static_cast<uint32_t>(-1))
        {
            std::cerr << "Failed to reload shader: " << name << "\n";
            return false;
        }

        program.Handle = newHandle;

        // Update last modified time
        try
        {
            if (program.IsCompute)
            {
                program.LastModified = std::filesystem::last_write_time(program.ComputePath);
            }
            else
            {
                auto vertTime = std::filesystem::last_write_time(program.VertexPath);
                auto fragTime = std::filesystem::last_write_time(program.FragmentPath);
                program.LastModified = std::max(vertTime, fragTime);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to get file modification time: " << e.what() << "\n";
        }

        std::cout << "Successfully reloaded shader: " << name << "\n";

        // Notify callbacks
        NotifyReloadCallbacks(name, newHandle);

        return true;
    }

    void ShaderManager::CheckForChanges()
    {
        if (!m_HotReloadEnabled)
            return;

        for (auto& [name, program] : m_Shaders)
        {
            if (CheckFileModified(program))
            {
                ReloadShader(name);
            }
        }
    }

    bool ShaderManager::CheckFileModified(ShaderProgram& program)
    {
        try
        {
            std::filesystem::file_time_type currentTime;

            if (program.IsCompute)
            {
                if (!std::filesystem::exists(program.ComputePath))
                    return false;
                currentTime = std::filesystem::last_write_time(program.ComputePath);
            }
            else
            {
                if (!std::filesystem::exists(program.VertexPath) ||
                    !std::filesystem::exists(program.FragmentPath))
                    return false;

                auto vertTime = std::filesystem::last_write_time(program.VertexPath);
                auto fragTime = std::filesystem::last_write_time(program.FragmentPath);
                currentTime = std::max(vertTime, fragTime);
            }

            return currentTime > program.LastModified;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error checking file modification: " << e.what() << "\n";
            return false;
        }
    }

    void ShaderManager::RegisterReloadCallback(const std::string& shaderName, ShaderReloadCallback callback)
    {
        m_ReloadCallbacks[shaderName].push_back(callback);
    }

    void ShaderManager::UnregisterReloadCallback(const std::string& shaderName)
    {
        m_ReloadCallbacks.erase(shaderName);
    }

    void ShaderManager::NotifyReloadCallbacks(const std::string& shaderName, uint32_t newHandle)
    {
        auto it = m_ReloadCallbacks.find(shaderName);
        if (it != m_ReloadCallbacks.end())
        {
            for (auto& callback : it->second)
            {
                callback(shaderName, newHandle);
            }
        }
    }

    void ShaderManager::Clear()
    {
        m_Shaders.clear();
        m_ReloadCallbacks.clear();
    }

    std::vector<std::string> ShaderManager::GetShaderNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_Shaders.size());

        for (const auto& [name, _] : m_Shaders)
        {
            names.push_back(name);
        }

        return names;
    }
    
    const ShaderProgram* ShaderManager::GetShaderInfo(const std::string& name) const
    {
        auto it = m_Shaders.find(name);
        if (it != m_Shaders.end())
        {
            return &it->second;
        }
        return nullptr;
    }
}
