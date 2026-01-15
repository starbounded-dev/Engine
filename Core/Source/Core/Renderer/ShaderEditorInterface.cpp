#include "ShaderEditorInterface.h"

namespace Core::Renderer
{
    // Static storage for the interface pointer
    static IShaderEditorInterface* s_ShaderEditorInterface = nullptr;

    IShaderEditorInterface* GetShaderEditorInterface()
    {
        return s_ShaderEditorInterface;
    }

    void SetShaderEditorInterface(IShaderEditorInterface* editor)
    {
        s_ShaderEditorInterface = editor;
    }
}
