# Material-Shader Editor Integration

## Overview

When you create a Material, its shaders are automatically loaded into the Shader Editor. This provides a seamless workflow for material creation and shader editing.

## Automatic Shader Loading

### On Material Creation

When you create a Material with shader paths, the shaders are automatically loaded into the Shader Editor:

```cpp
#include "Core/Renderer/Material.h"

// Creating a material automatically loads its shaders into the editor
auto material = std::make_shared<Material>(
    "App/Resources/Shaders/pbr.vert",
    "App/Resources/Shaders/pbr.frag"
);

// The Shader Editor now has these shaders loaded and ready to edit!
// Press F4 to open the Shader Editor and see them
```

### Manual Loading

You can also manually load a material's shaders into the editor at any time:

```cpp
// Later in your code, load the material's shaders into the editor
material->LoadIntoShaderEditor();
```

### From Material ImGui Editor

When displaying a material in ImGui (e.g., in an inspector panel), you'll see an "Edit Shaders" button:

```cpp
material->OnImGuiRender("My Material");
```

This shows:
- Shader file paths (VS and FS)
- **"Edit Shaders" button** - Click to load shaders into Shader Editor
- Material parameters with editors
- Texture bindings

## Complete Workflow Example

### 1. Application Setup

```cpp
// In your application initialization
void MyApp::OnAttach()
{
    // The ImLayer automatically sets up the ShaderEditor instance
    // No additional setup needed!
}
```

### 2. Create Materials

```cpp
// Create materials for your objects
auto groundMaterial = std::make_shared<Material>(
    "Resources/Shaders/terrain.vert",
    "Resources/Shaders/terrain.frag"
);

auto characterMaterial = std::make_shared<Material>(
    "Resources/Shaders/character.vert", 
    "Resources/Shaders/character.frag"
);

// Both materials' shaders are now accessible in the Shader Editor!
```

### 3. Edit Shaders

```cpp
// Option 1: Automatic on creation
// Materials automatically load their shaders when created

// Option 2: Manual loading
groundMaterial->LoadIntoShaderEditor();

// Option 3: Through ImGui inspector
if (ImGui::Begin("Inspector"))
{
    characterMaterial->OnImGuiRender("Character Material");
    // Click "Edit Shaders" button in the UI
}
ImGui::End();
```

### 4. Edit and See Changes

1. Press **F4** to open Shader Editor (or click "Edit Shaders" in material inspector)
2. Edit the vertex or fragment shader code
3. Press **Ctrl+S** to save
4. Changes are automatically reloaded if auto-reload is enabled
5. See your changes instantly in the viewport!

## Integration Details

### How It Works

1. **Material Creation**: When a `Material` is constructed, it calls `LoadIntoShaderEditor()`
2. **Global Access**: ShaderEditor has a global instance set by ImLayer
3. **Automatic Loading**: The material's vertex and fragment shader paths are loaded into the editor
4. **No Dependencies**: Uses external linkage to avoid circular dependencies between Core and App layers

### API Methods

#### Material Class

```cpp
class Material
{
public:
    // Get shader file paths
    const std::string& GetVertexPath() const;
    const std::string& GetFragmentPath() const;
    
    // Load this material's shaders into the shader editor
    void LoadIntoShaderEditor() const;
    
    // ImGui editor (includes "Edit Shaders" button)
    void OnImGuiRender(const char* label = nullptr);
};
```

#### ShaderEditor Class

```cpp
class ShaderEditor
{
public:
    // Load shader files for editing
    void LoadShaderFiles(const std::filesystem::path& vertexPath,
                        const std::filesystem::path& fragmentPath);
    
    // Global instance access
    static void SetInstance(ShaderEditor* instance);
    static ShaderEditor* GetInstance();
};
```

## Notes

- The ShaderEditor must be initialized (ImLayer does this automatically)
- Multiple materials can be created; the editor shows the last one loaded
- Use the shader list panel (F4) to see all registered shaders
- Materials created before ImLayer initialization won't auto-load (call `LoadIntoShaderEditor()` manually)

## Tips

- Create materials during or after ImLayer initialization for automatic loading
- Use material inspectors with `OnImGuiRender()` for easy shader editing access
- Enable "Auto-Reload on Save" in Shader Editor options for instant feedback
- The Shader Editor remembers your last loaded shaders between sessions

## Example: Material Inspector Panel

```cpp
void RenderMaterialInspector()
{
    ImGui::Begin("Material Inspector");
    
    if (selectedMaterial)
    {
        // Shows shader paths and "Edit Shaders" button
        selectedMaterial->OnImGuiRender("Selected Material");
        
        // Additional custom UI
        if (ImGui::Button("Duplicate Material"))
        {
            auto copy = std::make_shared<Material>(
                selectedMaterial->GetVertexPath(),
                selectedMaterial->GetFragmentPath()
            );
            // Copy also auto-loads its shaders!
        }
    }
    
    ImGui::End();
}
```

## Troubleshooting

**Q: Shaders don't load when material is created**
A: Ensure ImLayer is attached before creating materials, or call `LoadIntoShaderEditor()` manually after ImLayer initialization.

**Q: "Edit Shaders" button doesn't work**
A: Make sure the Shader Editor is enabled (F4) and the material has valid shader paths.

**Q: Multiple materials - which one is loaded?**
A: The last material that called `LoadIntoShaderEditor()` is shown. Use the shader list to switch between shaders.

**Q: Can I disable automatic loading?**
A: Yes, comment out the `LoadIntoShaderEditor()` call in the Material constructor if needed.
