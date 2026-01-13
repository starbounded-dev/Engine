# Renderer System Documentation

## Overview
This document describes the rendering system implementation in the Engine, covering all major features and systems.

## Core Systems (Priority: High)

### 1.1 Uniform Buffer Objects (UBO)
**Location**: `UniformBuffer.h/cpp`

Provides efficient uniform data management using OpenGL UBO system.

**Features**:
- Automatic uniform location caching
- Structured uniform data with reflection
- Per-frame, per-object, and per-material uniform buffers
- Standard layout (std140) support

**Usage**:
```cpp
// Create a UBO with a layout
UniformBufferLayout layout = UniformBufferLayout::Reflect(program, "FrameData");
UniformBuffer frameUBO(layout, UBOBinding::PerFrame);

// Update uniforms
frameUBO.SetMat4("u_ViewProjection", viewProjMatrix);
frameUBO.SetFloat("u_Time", time);
frameUBO.Upload(); // Upload to GPU
frameUBO.BindBase(); // Bind to binding point
```

### 1.2 Material System
**Location**: `Material.h/cpp`

Complete material system with shader integration and instancing support.

**Features**:
- Material resource sharing
- Material instancing for per-object overrides
- Texture binding management
- UBO-backed parameters
- ImGui material editor

**Usage**:
```cpp
// Create a material
Material material("shaders/pbr.vert", "shaders/pbr.frag");
material.SetVec3("u_Color", glm::vec3(1.0f, 0.0f, 0.0f));
material.SetTexture("u_Albedo", 0, albedoTexture);

// Create an instance with overrides
auto instance = material.CreateInstance();
instance->SetVec3("u_Color", glm::vec3(0.0f, 1.0f, 0.0f));

// Bind and render
instance->Bind();
// ... draw calls ...
```

### 1.3 Buffer Abstractions
**Location**: `Buffer.h/cpp`

Vertex and index buffer abstractions with automatic attribute binding.

**Features**:
- VertexBuffer with flexible layouts
- IndexBuffer with 16/32-bit indices
- Automatic VAO setup with DSA
- Dynamic and static buffer support

**Usage**:
```cpp
// Define vertex layout
VertexBufferLayout layout = {
    { ShaderDataType::Float3, "a_Position" },
    { ShaderDataType::Float3, "a_Normal" },
    { ShaderDataType::Float2, "a_TexCoord" }
};

// Create vertex buffer
VertexBuffer vbo(vertexData, vertexSize);
vbo.SetLayout(layout);

// Create index buffer
IndexBuffer ibo(indices, indexCount);
```

### 1.4 Mesh/Model System
**Location**: `Mesh.h/cpp`, `Model.h/cpp`

Mesh abstraction and model loading with assimp integration.

**Features**:
- Mesh class bundling VAO+VBO+IBO
- Model loading (OBJ, glTF, FBX, etc.)
- Model caching system
- Material index per mesh

**Usage**:
```cpp
// Load a model
auto model = Model::LoadCached("assets/models/sponza/Sponza.gltf");

// Render all meshes
model->Draw();
```

### 1.5 Camera System
**Location**: `Camera.h/cpp`

Complete camera system with multiple projection modes and controllers.

**Features**:
- Perspective and orthographic projection
- View matrix calculation
- FPS camera controller
- Orbit camera controller
- LookAt functionality

**Usage**:
```cpp
// Create camera
Camera camera;
camera.SetPerspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
camera.SetPosition(glm::vec3(0, 5, 10));

// FPS controller
FPSCameraController fpsController(&camera);
fpsController.OnUpdate(deltaTime, inputState);

// Get matrices
glm::mat4 viewProj = camera.GetViewProjectionMatrix();
```

## Medium Priority Systems

### 1.6 Render Command System
**Location**: `RenderCommand.h/cpp`, `RenderQueue.h/cpp`

Command buffer/queue pattern for deferred rendering and multi-threading.

**Features**:
- Command buffer recording
- Render queue for multi-threaded submission
- High-level commands (DrawMesh, SetMaterial, etc.)
- Automatic state management

**Usage**:
```cpp
RenderCommandBuffer cmdBuffer;
cmdBuffer.CmdSetPerFrame(viewProj, time, resolution);
cmdBuffer.CmdBindMaterial(&material);
cmdBuffer.CmdSetModelMatrix(transform);
cmdBuffer.CmdDrawMesh(&mesh);

// Submit to queue (can be from any thread)
renderQueue.Submit(std::move(cmdBuffer));

// Execute on render thread
renderQueue.Execute();
```

### 1.7 Batch Rendering
**Location**: `BatchRenderer.h/cpp`

2D sprite batching with texture atlases and instanced rendering.

**Features**:
- CPU-expanded and instanced rendering modes
- Texture batching (up to 32 textures)
- Sorting by texture for better batching
- Quad rendering with transforms

**Usage**:
```cpp
BatchRenderer2D batchRenderer;
batchRenderer.Init();

batchRenderer.BeginScene(viewProjection);
batchRenderer.DrawQuad(transform, textureID, color);
// ... more draw calls ...
batchRenderer.EndScene();
```

### 1.8 Lighting System
**Location**: `Light.h/cpp`

Comprehensive lighting system with shadow mapping support.

**Features**:
- Directional, point, and spot lights
- Shadow mapping support
- PBR-ready light data structures
- LightManager for scene lighting

**Usage**:
```cpp
// Create lights
Light dirLight(LightType::Directional);
dirLight.SetDirection(glm::vec3(0, -1, 0));
dirLight.SetColor(glm::vec3(1, 1, 1));
dirLight.SetIntensity(1.5f);
dirLight.SetCastsShadows(true);

// Add to manager
LightManager lightManager;
lightManager.AddLight(dirLight);

// Get GPU data for upload
auto lightData = lightManager.GetLightDataArray();
```

### 1.8 Shadow Mapping
**Location**: `ShadowMap.h/cpp`

Shadow map rendering for directional and point lights.

**Features**:
- 2D shadow maps for directional/spot lights
- Cubemap shadow maps for point lights
- ShadowMapManager for multiple lights
- Depth texture generation

**Usage**:
```cpp
ShadowMap shadowMap;
shadowMap.Create(1024, 1024);

// Render to shadow map
shadowMap.BindForWriting();
// ... render scene from light perspective ...
ShadowMap::UnbindFramebuffer();

// Use in shader
shadowMap.BindForReading(0); // Bind to texture unit 0
```

### 1.9 Render Pass System
**Location**: `RenderPass.h/cpp`

Render pass abstraction for organizing rendering pipeline.

**Features**:
- RenderTarget with multiple color attachments
- Clear operations
- Viewport management
- RenderPipeline for chaining passes

**Usage**:
```cpp
// Create render target
RenderTarget gbuffer;
std::vector<AttachmentDesc> attachments = {
    { GL_RGBA16F }, // Albedo
    { GL_RGBA16F }, // Normal
    { GL_RGBA16F }  // Position
};
gbuffer.Create(1920, 1080, attachments, true);

// Create render pass
auto geometryPass = std::make_shared<RenderPass>("Geometry");
geometryPass->SetRenderTarget(&gbuffer);
geometryPass->SetExecuteCallback([](RenderPass* pass) {
    // Render geometry here
});

// Create pipeline
RenderPipeline pipeline;
pipeline.AddPass(geometryPass);
pipeline.Execute();
```

### 1.9 Post-Processing
**Location**: `PostProcessing.h/cpp`

Post-processing effects framework.

**Features**:
- Bloom effect
- Tone mapping (Reinhard, ACES, Uncharted2)
- FXAA anti-aliasing
- PostProcessingStack for chaining

**Usage**:
```cpp
PostProcessingStack postStack;

auto bloom = std::make_shared<BloomEffect>();
bloom->SetThreshold(1.0f);
bloom->SetIntensity(0.5f);
postStack.AddEffect(bloom);

auto tonemap = std::make_shared<ToneMappingEffect>();
tonemap->SetMode(ToneMappingEffect::Mode::ACES);
postStack.AddEffect(tonemap);

postStack.Apply(sourceRT, destinationRT);
```

### 1.10 Shader Hot Reload
**Location**: `ShaderManager.h/cpp`

Centralized shader management with automatic hot reload.

**Features**:
- File modification tracking
- Automatic recompilation on change
- Error handling with rollback
- Reload callbacks for updates

**Usage**:
```cpp
auto& shaderMgr = ShaderManager::Get();

// Load shaders
uint32_t shader = shaderMgr.LoadGraphicsShader("PBR", 
    "shaders/pbr.vert", "shaders/pbr.frag");

// Register callback for reload
shaderMgr.RegisterReloadCallback("PBR", [&](const std::string& name, uint32_t newHandle) {
    // Update materials using this shader
    material.Rebuild();
});

// Check for changes (call each frame or on timer)
shaderMgr.CheckForChanges();
```

## Advanced Features (Priority: Low)

### 1.11 Compute Shader Pipeline
**Location**: `ComputeUtils.h/cpp`

Compute shader utilities and GPU computing support.

**Features**:
- SSBO (Shader Storage Buffer Objects)
- ComputeShader dispatcher
- GPU particle system framework
- GPU frustum culling

**Usage**:
```cpp
// Create SSBO
SSBO particleBuffer;
particleBuffer.Create(maxParticles * sizeof(ParticleData));

// Dispatch compute shader
ComputeShader computeShader(program);
computeShader.Dispatch(numGroupsX, numGroupsY, numGroupsZ);
ComputeShader::MemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
```

### 1.12 Advanced Textures
**Location**: `Texture.h/cpp`

Advanced texture support including cubemaps and 3D textures.

**Features**:
- 2D, 3D, Cubemap, and Array textures
- Texture loading from files
- EnvironmentMap for IBL
- TextureAtlas for batching

**Usage**:
```cpp
// Load cubemap
std::vector<std::filesystem::path> faces = {
    "right.jpg", "left.jpg", "top.jpg",
    "bottom.jpg", "front.jpg", "back.jpg"
};
Texture cubemap = Texture::LoadCubemapFromFiles(faces);

// Create 3D texture
Texture volume;
volume.Create3D(64, 64, 64, format, volumeData);
```

### 1.13 GPU-driven Rendering
**Location**: `ComputeUtils.h/cpp`

Indirect drawing and GPU culling support.

**Features**:
- IndirectDrawBuffer for indirect rendering
- Multi-draw indirect
- GPU frustum culling integration

**Usage**:
```cpp
// Setup indirect drawing
IndirectDrawBuffer indirectBuffer;
indirectBuffer.Create(maxDraws, true);

std::vector<IndirectDrawElementsCommand> commands;
// ... fill commands ...
indirectBuffer.SetCommands(commands.data(), commands.size());

// Draw
indirectBuffer.Draw(commands.size());
```

## File Structure

```
Core/Source/Core/Renderer/
├── Texture.h/cpp               - Advanced texture support
├── BatchRenderer.h/cpp         - 2D batch rendering
├── Buffer.h/cpp                - Vertex/Index buffers
├── Camera.h/cpp                - Camera system
├── ComputeUtils.h/cpp          - Compute shaders & GPU utilities
├── GLUtils.h/cpp               - OpenGL debug utilities
├── Light.h/cpp                 - Lighting system
├── Material.h/cpp              - Material system
├── Mesh.h/cpp                  - Mesh abstraction
├── Model.h/cpp                 - Model loading
├── PostProcessing.h/cpp        - Post-processing effects
├── RenderCommand.h/cpp         - Render commands
├── RenderPass.h/cpp            - Render passes
├── RenderQueue.h/cpp           - Command queue
├── Renderer.h/cpp              - Basic rendering utilities
├── Shader.h/cpp                - Shader compilation
├── ShaderManager.h/cpp         - Shader management & hot reload
├── ShadowMap.h/cpp             - Shadow mapping
└── UniformBuffer.h/cpp         - Uniform buffers
```

## Notes

- All systems use the `Core::Renderer` namespace
- Most classes follow RAII principles (move-only, automatic cleanup)
- OpenGL 4.5+ with DSA (Direct State Access) is used where possible
- Many advanced features include placeholder implementations for future enhancement
