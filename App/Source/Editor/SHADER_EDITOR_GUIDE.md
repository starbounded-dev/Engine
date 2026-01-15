# Shader Editor Usage Guide

## Automatically Loading Shaders

The Shader Editor integrates with the ShaderManager to automatically load registered shaders. Here's how to use it:

### Step 1: Register Shaders with ShaderManager

First, load your shaders using the ShaderManager (typically in your application initialization):

```cpp
#include "Core/Renderer/ShaderManager.h"

// In your initialization code
auto& shaderMgr = Core::Renderer::ShaderManager::Get();

// Load your shaders
shaderMgr.LoadGraphicsShader("BasicShader", 
    "App/Resources/Shaders/basic.vert", 
    "App/Resources/Shaders/basic.frag");

shaderMgr.LoadGraphicsShader("PBR", 
    "App/Resources/Shaders/pbr.vert", 
    "App/Resources/Shaders/pbr.frag");
```

### Step 2: Open Shader Editor

Press **F4** or go to **View > Shader Editor** to open the editor panel.

### Step 3: Select a Shader

The Shader Editor will automatically display all registered shaders in the left panel. Simply click on any shader name to load it into the editor.

## Features

### Automatic Loading
- All shaders registered with ShaderManager appear in the shader list
- Click any shader name to automatically load its vertex and fragment shader files
- The editor displays the current file paths at the top of each tab

### Editing
- **Vertex Shader Tab**: Edit vertex shader code
- **Fragment Shader Tab**: Edit fragment shader code
- Modified indicators (`*Modified*`) appear when changes are made

### Saving and Reloading
- **Ctrl+S** or **File > Save**: Save changes to disk
- **Ctrl+R** or **File > Reload**: Reload from disk (discards unsaved changes)
- **F5** or **File > Compile & Test**: Save and compile the shader

### Hot Reload Integration
- Enable **Auto-Reload on Save** in the Options menu
- When enabled, shaders are automatically recompiled when saved
- The ShaderManager's hot-reload system detects file changes and notifies the renderer

### Error Display
- Compilation errors appear in the error panel below the editor
- Errors are displayed with context to help you fix issues quickly

## Example Workflow

1. **Load a Shader**: Open Shader Editor (F4), click "BasicShader" in the list
2. **Edit**: Make changes to the vertex or fragment shader
3. **Save**: Press Ctrl+S to save (auto-reload happens if enabled)
4. **Test**: See your changes immediately in the viewport (if auto-reload is on)

## Keyboard Shortcuts

- **F4**: Toggle Shader Editor
- **Ctrl+S**: Save shader files
- **Ctrl+R**: Reload shader files
- **F5**: Compile and test shader
- **F1**: Toggle ImGui Demo (for reference)
- **F2**: Toggle Performance Overlay
- **F3**: Toggle Profiler Panel

## Notes

- Only graphics shaders (vertex + fragment) can be edited
- Compute shaders are not yet supported in the editor
- Large shader files (>16KB per file) will be truncated
- Auto-reload requires ShaderManager's hot-reload to be enabled

## Troubleshooting

**Q: Shader list is empty**
A: Make sure you've loaded shaders using `ShaderManager::LoadGraphicsShader()` before opening the editor.

**Q: Can't see my changes in the viewport**
A: Enable "Auto-Reload on Save" in Options menu, or manually press F5 to compile.

**Q: Getting "Compute shader editing not yet supported"**
A: The shader you selected is a compute shader. Only graphics shaders can be edited currently.

**Q: File won't save**
A: Check that the file paths are writable and that the shader files exist on disk.
