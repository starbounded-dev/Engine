#include "Viewport.h"
#include "Camera.h"
#include "Material.h"
#include "Core/Debug/Memory.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>

namespace Core::Renderer
{
    Viewport::Viewport(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height)
    {
        CreateFramebuffer();
        CreatePreviewMeshes();
        
        // Create default camera
        m_Camera = std::make_shared<Camera>();
        m_Camera->SetPerspective(45.0f, GetAspectRatio(), 0.1f, 100.0f);
        m_Camera->SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        m_Camera->LookAt(glm::vec3(0.0f));
    }

    Viewport::~Viewport()
    {
        DeleteFramebuffer();
        DeletePreviewMeshes();
    }

    void Viewport::CreateFramebuffer()
    {
        // Create framebuffer
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        
        // Create color attachment
        glGenTextures(1, &m_ColorAttachment);
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);
        
        // Track texture memory
        size_t colorSize = m_Width * m_Height * 4; // RGBA8
        Debug::Memory::TrackAllocation((void*)(uintptr_t)m_ColorAttachment, colorSize, 
                                      Debug::MemoryCategory::Framebuffer, "Viewport Color");
        
        // Create depth attachment
        glGenTextures(1, &m_DepthAttachment);
        glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Width, m_Height, 0, 
                    GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);
        
        // Track depth texture memory
        size_t depthSize = m_Width * m_Height * 4; // DEPTH24_STENCIL8
        Debug::Memory::TrackAllocation((void*)(uintptr_t)m_DepthAttachment, depthSize, 
                                      Debug::MemoryCategory::Framebuffer, "Viewport Depth");
        
        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            // Handle error (log, throw, etc.)
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Viewport::DeleteFramebuffer()
    {
        if (m_ColorAttachment)
        {
            Debug::Memory::TrackDeallocation((void*)(uintptr_t)m_ColorAttachment);
            glDeleteTextures(1, &m_ColorAttachment);
            m_ColorAttachment = 0;
        }
        
        if (m_DepthAttachment)
        {
            Debug::Memory::TrackDeallocation((void*)(uintptr_t)m_DepthAttachment);
            glDeleteTextures(1, &m_DepthAttachment);
            m_DepthAttachment = 0;
        }
        
        if (m_FBO)
        {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
    }

    void Viewport::CreatePreviewMeshes()
    {
        // Create sphere
        {
            std::vector<float> vertices;
            std::vector<uint32_t> indices;
            
            const int segments = 32;
            const int rings = 16;
            const float radius = 1.0f;
            
            // Generate sphere vertices
            for (int ring = 0; ring <= rings; ring++)
            {
                float phi = (float)ring / rings * glm::pi<float>();
                for (int seg = 0; seg <= segments; seg++)
                {
                    float theta = (float)seg / segments * 2.0f * glm::pi<float>();
                    
                    float x = radius * sin(phi) * cos(theta);
                    float y = radius * cos(phi);
                    float z = radius * sin(phi) * sin(theta);
                    
                    // Position
                    vertices.push_back(x);
                    vertices.push_back(y);
                    vertices.push_back(z);
                    
                    // Normal
                    vertices.push_back(x / radius);
                    vertices.push_back(y / radius);
                    vertices.push_back(z / radius);
                    
                    // UV
                    vertices.push_back((float)seg / segments);
                    vertices.push_back((float)ring / rings);
                }
            }
            
            // Generate sphere indices
            for (int ring = 0; ring < rings; ring++)
            {
                for (int seg = 0; seg < segments; seg++)
                {
                    int current = ring * (segments + 1) + seg;
                    int next = current + segments + 1;
                    
                    indices.push_back(current);
                    indices.push_back(next);
                    indices.push_back(current + 1);
                    
                    indices.push_back(current + 1);
                    indices.push_back(next);
                    indices.push_back(next + 1);
                }
            }
            
            m_SphereIndexCount = indices.size();
            
            // Create VAO, VBO, EBO
            glGenVertexArrays(1, &m_SphereVAO);
            glGenBuffers(1, &m_SphereVBO);
            glGenBuffers(1, &m_SphereEBO);
            
            glBindVertexArray(m_SphereVAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, m_SphereVBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_SphereEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
            
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            
            // UV
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            
            glBindVertexArray(0);
            
            // Track memory
            size_t vboSize = vertices.size() * sizeof(float);
            size_t eboSize = indices.size() * sizeof(uint32_t);
            Debug::Memory::TrackAllocation((void*)(uintptr_t)m_SphereVBO, vboSize, 
                                          Debug::MemoryCategory::Buffer, "Preview Sphere VBO");
            Debug::Memory::TrackAllocation((void*)(uintptr_t)m_SphereEBO, eboSize, 
                                          Debug::MemoryCategory::Buffer, "Preview Sphere EBO");
        }
        
        // Create cube
        {
            float cubeVertices[] = {
                // Position         // Normal           // UV
                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
                 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
                 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
                -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
                
                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
                 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
                 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
                
                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
                -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
                
                 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
                 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
                 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
                 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
                
                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
                 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
                 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
                
                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
                 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
                 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            };
            
            uint32_t cubeIndices[] = {
                0, 1, 2, 2, 3, 0,
                4, 5, 6, 6, 7, 4,
                8, 9, 10, 10, 11, 8,
                12, 13, 14, 14, 15, 12,
                16, 17, 18, 18, 19, 16,
                20, 21, 22, 22, 23, 20
            };
            
            m_CubeIndexCount = 36;
            
            glGenVertexArrays(1, &m_CubeVAO);
            glGenBuffers(1, &m_CubeVBO);
            glGenBuffers(1, &m_CubeEBO);
            
            glBindVertexArray(m_CubeVAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CubeEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
            
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            
            // UV
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            
            glBindVertexArray(0);
            
            // Track memory
            Debug::Memory::TrackAllocation((void*)(uintptr_t)m_CubeVBO, sizeof(cubeVertices), 
                                          Debug::MemoryCategory::Buffer, "Preview Cube VBO");
            Debug::Memory::TrackAllocation((void*)(uintptr_t)m_CubeEBO, sizeof(cubeIndices), 
                                          Debug::MemoryCategory::Buffer, "Preview Cube EBO");
        }
    }

    void Viewport::DeletePreviewMeshes()
    {
        if (m_SphereVAO)
        {
            glDeleteVertexArrays(1, &m_SphereVAO);
            Debug::Memory::TrackDeallocation((void*)(uintptr_t)m_SphereVBO);
            Debug::Memory::TrackDeallocation((void*)(uintptr_t)m_SphereEBO);
            glDeleteBuffers(1, &m_SphereVBO);
            glDeleteBuffers(1, &m_SphereEBO);
        }
        
        if (m_CubeVAO)
        {
            glDeleteVertexArrays(1, &m_CubeVAO);
            Debug::Memory::TrackDeallocation((void*)(uintptr_t)m_CubeVBO);
            Debug::Memory::TrackDeallocation((void*)(uintptr_t)m_CubeEBO);
            glDeleteBuffers(1, &m_CubeVBO);
            glDeleteBuffers(1, &m_CubeEBO);
        }
    }

    void Viewport::Resize(uint32_t width, uint32_t height)
    {
        if (m_Width == width && m_Height == height)
            return;
            
        m_Width = width;
        m_Height = height;
        
        DeleteFramebuffer();
        CreateFramebuffer();
        
        // Update camera aspect ratio
        if (m_Camera)
        {
            m_Camera->SetPerspective(45.0f, GetAspectRatio(), 0.1f, 100.0f);
        }
    }

    void Viewport::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glViewport(0, 0, m_Width, m_Height);
    }

    void Viewport::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Viewport::Clear(const glm::vec4& color) const
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Viewport::RenderPreviewSphere(std::shared_ptr<Material> material, float rotation)
    {
        if (!material || !m_Camera) return;
        
        Bind();
        Clear();
        
        glEnable(GL_DEPTH_TEST);
        
        // Create model matrix with rotation
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Bind material and set matrices
        material->Bind();
        material->SetMatrix4("u_Model", model);
        material->SetMatrix4("u_View", m_Camera->GetViewMatrix());
        material->SetMatrix4("u_Projection", m_Camera->GetProjectionMatrix());
        
        // Render sphere
        glBindVertexArray(m_SphereVAO);
        glDrawElements(GL_TRIANGLES, m_SphereIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        material->Unbind();
        Unbind();
    }

    void Viewport::RenderPreviewCube(std::shared_ptr<Material> material, float rotation)
    {
        if (!material || !m_Camera) return;
        
        Bind();
        Clear();
        
        glEnable(GL_DEPTH_TEST);
        
        // Create model matrix with rotation
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.5f, 1.0f, 0.3f));
        
        // Bind material and set matrices
        material->Bind();
        material->SetMatrix4("u_Model", model);
        material->SetMatrix4("u_View", m_Camera->GetViewMatrix());
        material->SetMatrix4("u_Projection", m_Camera->GetProjectionMatrix());
        
        // Render cube
        glBindVertexArray(m_CubeVAO);
        glDrawElements(GL_TRIANGLES, m_CubeIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        material->Unbind();
        Unbind();
    }
}
