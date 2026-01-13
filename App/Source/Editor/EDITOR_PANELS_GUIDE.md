# Editor Panels Guide

This guide provides an overview of all editor panels available in the Engine, their features, and how to use them.

## Overview

The Engine includes 6 fully-featured editor panels, all accessible via keyboard shortcuts (F1-F6) and the View/Tools menu:

1. **ImGui Demo** (F1) - ImGui demonstration window
2. **Performance Overlay** (F2) - Floating FPS counter
3. **Profiler Panel** (F3) - Performance profiling and Tracy integration
4. **Shader Editor** (F4) - Shader editing with hot-reload
5. **Renderer Stats Panel** (F5) - Comprehensive renderer statistics
6. **Material Editor** (F6) - Visual material editing

All panels are fully dockable and support the modern dark theme with blue accents.

---

## 1. Performance Overlay (F2)

**Location**: Floating in top-right corner

**Features**:
- Real-time FPS display
- Frame time in milliseconds
- Keyboard shortcut reference
- Minimal, non-intrusive design

**Usage**:
```cpp
// Toggle via keyboard
Press F2

// Or programmatically
m_ShowOverlay = !m_ShowOverlay;
```

---

## 2. Profiler Panel (F3)

**Location**: `App/Source/Editor/ProfilerPanel.h/cpp`

**Features**:
- **Tracy Profiler Integration**: Connection status and profiler info
- **Frame Time Graph**: 120-frame history visualization
- **Memory Profiling**: Allocated, used, and free memory with progress bars
- **Rendering Statistics**: Draw calls, triangles, vertices
- **Custom Metrics**: Add your own performance metrics
- **Display Toggles**: Show/hide individual sections

**Usage**:
```cpp
// Update profiler metrics each frame
Editor::PerformanceMetrics metrics;
metrics.FrameTime = deltaTime * 1000.0f; // ms
metrics.FPS = 1.0f / deltaTime;
metrics.DrawCalls = rendererStats.drawCalls;
// ... set other metrics

profilerPanel->UpdateMetrics(metrics);

// Add custom metrics
profilerPanel->AddCustomMetric("GPU Time", gpuTime);
profilerPanel->AddCustomMetric("Physics Time", physicsTime);
```

**Display Sections**:
- Frame time graph with min/avg/max
- Memory info (allocated/used/free)
- Rendering stats (draws/tris/verts)
- Custom metrics list
- Tracy profiler status

---

## 3. Shader Editor (F4)

**Location**: `App/Source/Editor/ShaderEditor.h/cpp`

**Features**:
- **Automatic Shader Loading**: From ShaderManager and Materials
- **Tabbed Interface**: Separate vertex and fragment shader tabs
- **Live Editing**: Edit shaders directly in the editor
- **File Operations**: Save (Ctrl+S), Reload (Ctrl+R), Compile (F5)
- **Error Display**: Shows compilation errors
- **Shader List**: All registered shaders appear automatically
- **Hot-Reload Integration**: Auto-reload on save option
- **Material Integration**: "Edit Shaders" button in Material inspector

**Usage**:
```cpp
// Shaders load automatically when registered
auto& shaderMgr = Core::Renderer::ShaderManager::Get();
shaderMgr.LoadGraphicsShader("PBR", "pbr.vert", "pbr.frag");
// Press F4, click "PBR" in list - shaders load automatically!

// Or from a Material
auto material = std::make_shared<Material>("pbr.vert", "pbr.frag");
// Shaders already loaded! Press F4 to edit

// Manual loading
material->LoadIntoShaderEditor();
```

**Keyboard Shortcuts**:
- **Ctrl+S**: Save current shader
- **Ctrl+R**: Reload from disk
- **F5**: Compile shader

See `SHADER_EDITOR_GUIDE.md` for complete documentation.

---

## 4. Renderer Stats Panel (F5)

**Location**: `App/Source/Editor/StatsPanel.h/cpp`

**Features**:
- **Draw Call Statistics**: 
  - Draw calls, triangles, vertices (current + peak)
  - Average triangles per draw call
  - Triangles per second
- **Memory Statistics**:
  - Texture memory (used/allocated with progress bar)
  - Buffer memory (vertex/index/uniform buffers)
  - Smart formatting (B, KB, MB, GB)
- **Frame Time Graph**:
  - 120-frame history
  - Min/avg/max display
  - Color-coded 60fps/30fps targets
- **Render Pass Info**:
  - Render passes count
  - Shader switches
  - Efficiency metrics (draws per pass, draws per shader)
- **Display Options**:
  - Toggle individual sections
  - Reset peak values button

**Usage**:
```cpp
// Update stats each frame
Editor::RendererStats stats;
stats.DrawCalls = myRenderer.GetDrawCallCount();
stats.TriangleCount = myRenderer.GetTriangleCount();
stats.VertexCount = myRenderer.GetVertexCount();
stats.TextureMemoryUsed = textureManager.GetUsedMemory();
stats.TextureMemoryAllocated = textureManager.GetAllocatedMemory();
stats.TextureCount = textureManager.GetTextureCount();
stats.FrameTime = deltaTime * 1000.0f;
stats.FPS = 1.0f / deltaTime;
stats.RenderPasses = pipeline.GetPassCount();
stats.ShaderSwitches = renderer.GetShaderSwitchCount();

statsPanel->UpdateStats(stats);

// Reset peak tracking
statsPanel->ResetStats();
```

**Display Sections**:
- Draw call statistics with derived metrics
- Memory statistics with progress bars
- Frame time graph with performance targets
- Render pass efficiency metrics

---

## 5. Material Editor (F6)

**Location**: `App/Source/Editor/MaterialEditor.h/cpp`

**Features**:
- **Visual Material Editor**: Edit all material properties visually
- **Tabbed Interface**: Properties, Textures, Preview, Actions
- **Texture Slot Assignment**: Add/remove/load texture slots
- **Property Tweaking**: Live editing with appropriate widgets:
  - Float: Drag slider
  - Vec3: Color picker or drag sliders
  - Vec4: Color picker with alpha or drag sliders
- **Material Template System**: Pre-defined templates for quick creation:
  - **Unlit**: Basic color + texture
  - **PBR**: Albedo, metallic, roughness, AO maps
  - **Standard**: Diffuse, specular, normal maps
- **Live Preview**: Placeholder for 3D preview rendering
- **Material Library**: Manage multiple materials
- **Shader Integration**: "Edit Shaders" button loads material's shaders in Shader Editor

**Usage**:
```cpp
// Create material from template
materialEditor->OnImGuiRender();
// Click "New Material", select template, create!

// Set material to edit
auto material = std::make_shared<Material>("pbr.vert", "pbr.frag");
materialEditor->SetMaterial(material);

// Add custom template
Editor::MaterialTemplate customTemplate;
customTemplate.Name = "MyCustom";
customTemplate.VertexShader = "custom.vert";
customTemplate.FragmentShader = "custom.frag";
customTemplate.DefaultValues["u_CustomParam"] = 1.0f;
customTemplate.TextureSlots = { "u_CustomTexture" };
materialEditor->AddTemplate(customTemplate);

// Enable/disable live preview
materialEditor->SetLivePreview(true);
```

**Tabs**:

1. **Properties**: Edit all material parameters with appropriate widgets
2. **Textures**: Manage texture slots, load textures (placeholder)
3. **Preview**: 3D preview viewport (rendering not yet implemented)
4. **Actions**: Rebuild shader, edit shaders, clone material

**Material Templates**:
- **Unlit**: Simple unlit material with color and texture
- **PBR**: Physically-based rendering with full material properties
- **Standard**: Phong-style material with diffuse/specular/normal maps

---

## Integration with ImLayer

All panels are integrated into the main ImLayer class and accessible via:

### Keyboard Shortcuts
- **F1**: ImGui Demo
- **F2**: Performance Overlay
- **F3**: Profiler Panel
- **F4**: Shader Editor
- **F5**: Renderer Stats Panel
- **F6**: Material Editor

### Menu System
- **View Menu**: Toggle visibility of all panels
- **Tools Menu**: Quick access to all panels + Reset Layout

### Docking
All panels are fully dockable within the workspace. Drag panel tabs to:
- Dock to edges
- Create tabbed groups
- Split into new areas
- Float independently

Use **Tools > Reset Layout** to restore default docking configuration.

---

## Styling

All panels use the consistent modern dark theme:
- **Background**: Dark gray (#1E1E1E)
- **Accent**: Blue (#2695E2)
- **Text**: White/Light gray
- **Borders**: Rounded corners (5px)
- **Spacing**: Consistent padding throughout

---

## Performance Considerations

### Profiler Panel
- Updates once per frame
- History buffer: 120 frames
- Low overhead: ~0.1ms per frame

### Stats Panel
- Updates once per frame
- History buffer: 120 frames
- Low overhead: ~0.1ms per frame

### Shader Editor
- Only updates when editing
- File I/O is asynchronous
- Hot-reload triggers on file modification

### Material Editor
- Live preview disabled by default
- Properties update on edit only
- Material library uses shared pointers for efficiency

---

## Tips & Best Practices

1. **Use Keyboard Shortcuts**: F1-F6 for quick panel access
2. **Dock Your Workspace**: Organize panels for your workflow
3. **Monitor Performance**: Keep Stats Panel visible during development
4. **Track Peak Values**: Reset peak values in Stats Panel to measure specific operations
5. **Use Material Templates**: Start with templates and customize
6. **Edit Shaders Quickly**: Use "Edit Shaders" button in Material Editor for instant access
7. **Profile Regularly**: Use Profiler Panel to identify bottlenecks
8. **Save Layouts**: Your docking layout persists between sessions

---

## Future Enhancements

Planned features for future releases:

### Stats Panel
- Real-time GPU memory tracking
- Texture atlas visualization
- Batch rendering statistics
- GPU performance counters

### Material Editor
- Actual 3D preview rendering
- Material saving/loading (serialization)
- Material graph editor
- Shader node connections visualization
- Material instancing UI

### Shader Editor
- Syntax highlighting
- Code completion
- Shader includes support
- Compute shader editing

### General
- Custom panel layouts
- Panel state persistence
- Plugin system for custom panels
- Remote profiling support

---

## Troubleshooting

### Panel Not Showing
- Check visibility with F-key shortcut
- Check View menu
- Try Tools > Reset Layout

### Stats Not Updating
- Ensure `UpdateStats()` is called each frame
- Check if panel is enabled: `statsPanel->SetEnabled(true)`

### Material Editor Empty
- Create or load a material first
- Click "New Material" button
- Or use `materialEditor->SetMaterial(material)`

### Shader Editor Not Loading Shaders
- Ensure ShaderManager has registered shaders
- Check file paths are correct
- Verify ShaderEditor instance is registered in ImLayer

---

## API Reference

### StatsPanel
```cpp
class StatsPanel
{
    void OnImGuiRender();
    void UpdateStats(const RendererStats& stats);
    void ResetStats();
    void SetEnabled(bool enabled);
    bool IsEnabled() const;
};
```

### MaterialEditor
```cpp
class MaterialEditor
{
    void OnImGuiRender();
    void SetMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> GetMaterial() const;
    void AddTemplate(const MaterialTemplate& tmpl);
    void SetEnabled(bool enabled);
    void SetLivePreview(bool enabled);
};
```

### ProfilerPanel
```cpp
class ProfilerPanel
{
    void OnImGuiRender();
    void UpdateMetrics(const PerformanceMetrics& metrics);
    void AddCustomMetric(const std::string& name, float value);
    void SetEnabled(bool enabled);
};
```

### ShaderEditor
```cpp
class ShaderEditor
{
    void OnImGuiRender();
    void LoadShaderFiles(const std::string& vertPath, const std::string& fragPath);
    void SetEnabled(bool enabled);
    static void SetInstance(ShaderEditor* instance);
    static ShaderEditor* GetInstance();
};
```

---

## See Also

- `SHADER_EDITOR_GUIDE.md` - Detailed shader editor documentation
- `MATERIAL_SHADER_INTEGRATION.md` - Material-shader editor integration
- `Core/Source/Core/Renderer/README.md` - Renderer API documentation
- `IMPLEMENTATION_SUMMARY.md` - Technical implementation details
