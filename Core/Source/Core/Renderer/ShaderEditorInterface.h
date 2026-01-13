#pragma once
#include <filesystem>

namespace Core::Renderer
{
    // Interface to avoid circular dependencies between Core and App layers
    // This allows Material (in Core) to communicate with ShaderEditor (in App)
    // without including ShaderEditor header or creating circular dependencies
    class IShaderEditorInterface
    {
    public:
        virtual ~IShaderEditorInterface() = default;
        virtual void LoadShaderFiles(const std::filesystem::path& vertexPath, 
                                    const std::filesystem::path& fragmentPath) = 0;
    };

    // Global accessor - implemented in App layer
    IShaderEditorInterface* GetShaderEditorInterface();
    void SetShaderEditorInterface(IShaderEditorInterface* editor);
}
