#include "Material.h"
#include "ShaderEditorInterface.h"

#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

// ImGui (you already have it if you have ImGuiLayer)
#include <imgui.h>

namespace Core::Renderer
{
    static std::string ReadTextFile(const std::string& path)
    {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in)
            return {};

        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    static GLuint Compile(GLenum stage, const std::string& src, const char* debugName)
    {
        GLuint sh = glCreateShader(stage);
        const char* cstr = src.c_str();
        glShaderSource(sh, 1, &cstr, nullptr);
        glCompileShader(sh);

        GLint ok = 0;
        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
            std::string log((size_t)len, '\0');
            glGetShaderInfoLog(sh, len, &len, log.data());

            glDeleteShader(sh);
            // Replace with your logger
            assert(false && "Shader compile failed");
        }

        return sh;
    }

    static GLuint LinkProgram(GLuint vs, GLuint fs)
    {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);

        GLint ok = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log((size_t)len, '\0');
            glGetProgramInfoLog(prog, len, &len, log.data());

            glDeleteProgram(prog);
            assert(false && "Program link failed");
        }

        glDetachShader(prog, vs);
        glDetachShader(prog, fs);
        return prog;
    }

    // ---------- Material ----------

    Material::Material(const std::string& vertexPath, const std::string& fragmentPath)
    {
        m_Res = std::make_shared<MaterialResources>();
        m_Res->VertexPath = vertexPath;
        m_Res->FragmentPath = fragmentPath;
        Rebuild();
        
        // Automatically load this material's shaders into the shader editor (if available)
        LoadIntoShaderEditor();
    }

    Material::~Material()
    {
        Destroy();
    }

    Material::Material(Material&& other) noexcept
    {
        *this = std::move(other);
    }

    Material& Material::operator=(Material&& other) noexcept
    {
        if (this == &other) return *this;

        Destroy();

        m_Res = std::move(other.m_Res);
        m_MaterialUBO = std::move(other.m_MaterialUBO);
        m_Values = std::move(other.m_Values);
        m_Textures = std::move(other.m_Textures);

        return *this;
    }

    void Material::Destroy()
    {
        if (m_Res && m_Res->Program)
        {
            glDeleteProgram(m_Res->Program);
            m_Res->Program = 0;
        }
    }

    GLuint Material::GetProgram() const
    {
        return m_Res ? m_Res->Program : 0;
    }

    void Material::Rebuild()
    {
        const std::string vsrc = ReadTextFile(m_Res->VertexPath);
        const std::string fsrc = ReadTextFile(m_Res->FragmentPath);
        assert(!vsrc.empty() && !fsrc.empty());

        GLuint vs = Compile(GL_VERTEX_SHADER, vsrc, m_Res->VertexPath.c_str());
        GLuint fs = Compile(GL_FRAGMENT_SHADER, fsrc, m_Res->FragmentPath.c_str());
        GLuint prog = LinkProgram(vs, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);

        if (m_Res->Program)
            glDeleteProgram(m_Res->Program);
        m_Res->Program = prog;

        // Reflect material block
        m_Res->MaterialLayout = UniformBufferLayout::Reflect(prog, m_Res->MaterialBlockName);

        // Create per-material UBO if block exists
        if (m_Res->MaterialLayout.GetSize() > 0)
        {
            m_MaterialUBO = UniformBuffer(m_Res->MaterialLayout, UBOBinding::PerMaterial, true);

            // Seed default values map from reflection (zero init)
            // (We don�t know �nice defaults� automatically, but editor needs keys.)
            // Also: some drivers return "MaterialData.x" -> keep that key too.
            // We'll normalize in ResolveMaterialUBOName().
            // Grab keys:
            // (Layout doesn't expose iteration; so you can either extend it later,
            //  or just rely on Set* calls to populate m_Values.)
        }
    }

    bool Material::HasMaterialUBO(const std::string& name) const
    {
        if (m_Res->MaterialLayout.GetSize() == 0) return false;
        return m_Res->MaterialLayout.Find(name) != nullptr
            || m_Res->MaterialLayout.Find(m_Res->MaterialBlockName + "." + name) != nullptr;
    }

    std::string Material::ResolveMaterialUBOName(const std::string& name) const
    {
        if (m_Res->MaterialLayout.Find(name))
            return name;

        std::string alt = m_Res->MaterialBlockName + "." + name;
        if (m_Res->MaterialLayout.Find(alt))
            return alt;

        return name; // fallback (will go to glUniform path)
    }

    GLint Material::GetUniformLocationCached(const std::string& name) const
    {
        auto& cache = m_Res->UniformLocationCache;
        auto it = cache.find(name);
        if (it != cache.end())
            return it->second;

        GLint loc = glGetUniformLocation(m_Res->Program, name.c_str());
        cache[name] = loc;
        return loc;
    }

    void Material::SetUniformFallback(const std::string& name, const MaterialValue& v) const
    {
        GLint loc = GetUniformLocationCached(name);
        if (loc < 0) return;

        glUseProgram(m_Res->Program);

        std::visit([&](auto&& val)
            {
                using T = std::decay_t<decltype(val)>;

                if constexpr (std::is_same_v<T, float>)        glUniform1f(loc, val);
                else if constexpr (std::is_same_v<T, int32_t>) glUniform1i(loc, val);
                else if constexpr (std::is_same_v<T, uint32_t>)glUniform1ui(loc, val);
                else if constexpr (std::is_same_v<T, glm::vec2>) glUniform2fv(loc, 1, glm::value_ptr(val));
                else if constexpr (std::is_same_v<T, glm::vec3>) glUniform3fv(loc, 1, glm::value_ptr(val));
                else if constexpr (std::is_same_v<T, glm::vec4>) glUniform4fv(loc, 1, glm::value_ptr(val));
                else if constexpr (std::is_same_v<T, glm::mat3>) glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
                else if constexpr (std::is_same_v<T, glm::mat4>) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
            }, v);
    }

    void Material::SetFloat(const std::string& name, float v)
    {
        m_Values[name] = v;

        if (HasMaterialUBO(name))
            m_MaterialUBO.SetFloat(ResolveMaterialUBOName(name), v, true);
        else
            SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetInt(const std::string& name, int32_t v)
    {
        m_Values[name] = v;

        if (HasMaterialUBO(name))
            m_MaterialUBO.SetInt(ResolveMaterialUBOName(name), v, true);
        else
            SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetUInt(const std::string& name, uint32_t v)
    {
        m_Values[name] = v;

        if (HasMaterialUBO(name))
            m_MaterialUBO.SetUInt(ResolveMaterialUBOName(name), v, true);
        else
            SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetVec2(const std::string& name, const glm::vec2& v)
    {
        m_Values[name] = v;
        if (HasMaterialUBO(name))
        {
            float tmp[2] = { v.x, v.y };
            m_MaterialUBO.SetVec2(ResolveMaterialUBOName(name), tmp, true);
        }
        else SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetVec3(const std::string& name, const glm::vec3& v)
    {
        m_Values[name] = v;
        if (HasMaterialUBO(name))
        {
            float tmp[3] = { v.x, v.y, v.z };
            m_MaterialUBO.SetVec3(ResolveMaterialUBOName(name), tmp, true);
        }
        else SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetVec4(const std::string& name, const glm::vec4& v)
    {
        m_Values[name] = v;
        if (HasMaterialUBO(name))
        {
            float tmp[4] = { v.x, v.y, v.z, v.w };
            m_MaterialUBO.SetVec4(ResolveMaterialUBOName(name), tmp, true);
        }
        else SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetMat3(const std::string& name, const glm::mat3& v)
    {
        m_Values[name] = v;
        if (HasMaterialUBO(name))
            m_MaterialUBO.SetMat3(ResolveMaterialUBOName(name), glm::value_ptr(v), true);
        else
            SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetMat4(const std::string& name, const glm::mat4& v)
    {
        m_Values[name] = v;
        if (HasMaterialUBO(name))
            m_MaterialUBO.SetMat4(ResolveMaterialUBOName(name), glm::value_ptr(v), true);
        else
            SetUniformFallback(name, m_Values[name]);
    }

    void Material::SetTexture(const std::string& samplerUniform, uint32_t slot, GLuint textureID, GLenum target)
    {
        // Update or add
        auto it = std::find_if(m_Textures.begin(), m_Textures.end(),
            [&](const TextureBinding& b) { return b.Uniform == samplerUniform; });

        if (it == m_Textures.end())
            m_Textures.push_back({ samplerUniform, slot, textureID, target });
        else
        {
            it->Slot = slot;
            it->TextureID = textureID;
            it->Target = target;
        }
    }

    void Material::BindTextures() const
    {
        for (const auto& t : m_Textures)
        {
            if (t.TextureID == 0) continue;

            // bind texture id to unit
            glBindTextureUnit(t.Slot, t.TextureID);

            // set sampler uniform to the unit (cached)
            GLint loc = GetUniformLocationCached(t.Uniform);
            if (loc >= 0)
                glUniform1i(loc, (GLint)t.Slot);
        }
    }

    void Material::Bind() const
    {
        glUseProgram(m_Res->Program);

        // Bind per-material UBO (binding = 2) if present
        if (m_Res->MaterialLayout.GetSize() > 0)
            m_MaterialUBO.BindBase();

        // Bind textures + sampler uniforms
        BindTextures();
    }

    std::shared_ptr<MaterialInstance> Material::CreateInstance() const
    {
        return std::make_shared<MaterialInstance>(std::shared_ptr<const Material>(m_Res ? this->shared_from_this() : nullptr));
    }

    // NOTE: If you don't have enable_shared_from_this on Material, use this safer version:
    // (I�ll keep it simple below by implementing instance ctor from raw program path.)
    // You can change CreateInstance() to:
    // return std::make_shared<MaterialInstance>(std::make_shared<Material>(*this));  // deep copy (not ideal)
    // Better: make Material inherit enable_shared_from_this<Material>.

    void Material::OnImGuiRender(const char* label)
    {
        const char* hdr = label ? label : "Material";
        if (!ImGui::CollapsingHeader(hdr))
            return;

        ImGui::Text("Shader:");
        ImGui::BulletText("VS: %s", m_Res->VertexPath.c_str());
        ImGui::BulletText("FS: %s", m_Res->FragmentPath.c_str());
        
        // Button to load shaders into shader editor
        if (ImGui::Button("Edit Shaders"))
        {
            LoadIntoShaderEditor();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(F4 to open Shader Editor)");

        if (ImGui::TreeNode("Parameters"))
        {
            for (auto& [name, val] : m_Values)
            {
                // Heuristic: treat *Color* as color
                bool isColor = (name.find("Color") != std::string::npos) || (name.find("color") != std::string::npos);

                std::visit([&](auto&& v)
                    {
                        using T = std::decay_t<decltype(v)>;

                        if constexpr (std::is_same_v<T, float>)
                        {
                            float tmp = v;
                            if (ImGui::DragFloat(name.c_str(), &tmp, 0.01f))
                                SetFloat(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, int32_t>)
                        {
                            int tmp = (int)v;
                            if (ImGui::DragInt(name.c_str(), &tmp, 1.0f))
                                SetInt(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, uint32_t>)
                        {
                            int tmp = (int)v;
                            if (ImGui::DragInt(name.c_str(), &tmp, 1.0f))
                                SetUInt(name, (uint32_t)std::max(tmp, 0));
                        }
                        else if constexpr (std::is_same_v<T, glm::vec2>)
                        {
                            glm::vec2 tmp = v;
                            if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(tmp), 0.01f))
                                SetVec2(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec3>)
                        {
                            glm::vec3 tmp = v;
                            bool changed = isColor
                                ? ImGui::ColorEdit3(name.c_str(), glm::value_ptr(tmp))
                                : ImGui::DragFloat3(name.c_str(), glm::value_ptr(tmp), 0.01f);
                            if (changed) SetVec3(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec4>)
                        {
                            glm::vec4 tmp = v;
                            bool changed = isColor
                                ? ImGui::ColorEdit4(name.c_str(), glm::value_ptr(tmp))
                                : ImGui::DragFloat4(name.c_str(), glm::value_ptr(tmp), 0.01f);
                            if (changed) SetVec4(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, glm::mat3>)
                        {
                            ImGui::Text("%s (mat3)", name.c_str());
                        }
                        else if constexpr (std::is_same_v<T, glm::mat4>)
                        {
                            ImGui::Text("%s (mat4)", name.c_str());
                        }
                    }, val);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Textures"))
        {
            for (auto& t : m_Textures)
            {
                ImGui::Text("%s -> slot %u (id %u)", t.Uniform.c_str(), t.Slot, (unsigned)t.TextureID);
            }
            ImGui::TreePop();
        }
    }

    // ---------- MaterialInstance ----------
    // NOTE: For clean sharing, make Material inherit std::enable_shared_from_this<Material>.
    // Here we keep it simple: instance stores base pointer and clones UBO layout.

    MaterialInstance::MaterialInstance(std::shared_ptr<const Material> base)
        : m_Base(std::move(base))
    {
        assert(m_Base);

        const auto& layout = m_Base->m_Res->MaterialLayout;
        if (layout.GetSize() > 0)
            m_InstanceUBO = UniformBuffer(layout, UBOBinding::PerMaterial, true);
    }

    bool MaterialInstance::HasMaterialUBO(const std::string& name) const
    {
        const auto& layout = m_Base->m_Res->MaterialLayout;
        if (layout.GetSize() == 0) return false;

        return layout.Find(name) != nullptr
            || layout.Find(m_Base->m_Res->MaterialBlockName + "." + name) != nullptr;
    }

    std::string MaterialInstance::ResolveMaterialUBOName(const std::string& name) const
    {
        const auto& layout = m_Base->m_Res->MaterialLayout;
        if (layout.Find(name)) return name;

        std::string alt = m_Base->m_Res->MaterialBlockName + "." + name;
        if (layout.Find(alt)) return alt;

        return name;
    }

    void MaterialInstance::SetFloat(const std::string& name, float v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
            m_InstanceUBO.SetFloat(ResolveMaterialUBOName(name), v, true);
        else
            m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetInt(const std::string& name, int32_t v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
            m_InstanceUBO.SetInt(ResolveMaterialUBOName(name), v, true);
        else
            m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetUInt(const std::string& name, uint32_t v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
            m_InstanceUBO.SetUInt(ResolveMaterialUBOName(name), v, true);
        else
            m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetVec2(const std::string& name, const glm::vec2& v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
        {
            float tmp[2] = { v.x, v.y };
            m_InstanceUBO.SetVec2(ResolveMaterialUBOName(name), tmp, true);
        }
        else m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetVec3(const std::string& name, const glm::vec3& v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
        {
            float tmp[3] = { v.x, v.y, v.z };
            m_InstanceUBO.SetVec3(ResolveMaterialUBOName(name), tmp, true);
        }
        else m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetVec4(const std::string& name, const glm::vec4& v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
        {
            float tmp[4] = { v.x, v.y, v.z, v.w };
            m_InstanceUBO.SetVec4(ResolveMaterialUBOName(name), tmp, true);
        }
        else m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetMat3(const std::string& name, const glm::mat3& v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
            m_InstanceUBO.SetMat3(ResolveMaterialUBOName(name), glm::value_ptr(v), true);
        else
            m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetMat4(const std::string& name, const glm::mat4& v)
    {
        m_Overrides[name] = v;
        if (HasMaterialUBO(name))
            m_InstanceUBO.SetMat4(ResolveMaterialUBOName(name), glm::value_ptr(v), true);
        else
            m_Base->SetUniformFallback(name, m_Overrides[name]);
    }

    void MaterialInstance::SetTexture(const std::string& samplerUniform, uint32_t slot, GLuint textureID, GLenum target)
    {
        auto it = std::find_if(m_TextureOverrides.begin(), m_TextureOverrides.end(),
            [&](const TextureBinding& b) { return b.Uniform == samplerUniform; });

        if (it == m_TextureOverrides.end())
            m_TextureOverrides.push_back({ samplerUniform, slot, textureID, target });
        else
        {
            it->Slot = slot;
            it->TextureID = textureID;
            it->Target = target;
        }
    }

    void MaterialInstance::BindTextures() const
    {
        // Base textures first
        for (const auto& t : m_Base->GetTextures())
        {
            if (t.TextureID == 0) continue;
            glBindTextureUnit(t.Slot, t.TextureID);
            GLint loc = m_Base->GetUniformLocationCached(t.Uniform);
            if (loc >= 0) glUniform1i(loc, (GLint)t.Slot);
        }

        // Overrides
        for (const auto& t : m_TextureOverrides)
        {
            if (t.TextureID == 0) continue;
            glBindTextureUnit(t.Slot, t.TextureID);
            GLint loc = m_Base->GetUniformLocationCached(t.Uniform);
            if (loc >= 0) glUniform1i(loc, (GLint)t.Slot);
        }
    }

    void MaterialInstance::Bind() const
    {
        glUseProgram(m_Base->GetProgram());

        if (m_Base->m_Res->MaterialLayout.GetSize() > 0)
            m_InstanceUBO.BindBase();

        BindTextures();
    }

    void MaterialInstance::OnImGuiRender(const char* label)
    {
        const char* hdr = label ? label : "MaterialInstance";
        if (!ImGui::CollapsingHeader(hdr))
            return;

        if (ImGui::TreeNode("Overrides"))
        {
            for (auto& [name, val] : m_Overrides)
            {
                std::visit([&](auto&& v)
                    {
                        using T = std::decay_t<decltype(v)>;

                        if constexpr (std::is_same_v<T, float>)
                        {
                            float tmp = v;
                            if (ImGui::DragFloat(name.c_str(), &tmp, 0.01f))
                                SetFloat(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, int32_t>)
                        {
                            int tmp = (int)v;
                            if (ImGui::DragInt(name.c_str(), &tmp, 1.0f))
                                SetInt(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec2>)
                        {
                            glm::vec2 tmp = v;
                            if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(tmp), 0.01f))
                                SetVec2(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec3>)
                        {
                            glm::vec3 tmp = v;
                            if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(tmp), 0.01f))
                                SetVec3(name, tmp);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec4>)
                        {
                            glm::vec4 tmp = v;
                            if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(tmp), 0.01f))
                                SetVec4(name, tmp);
                        }
                        else
                        {
                            ImGui::Text("%s (non-editable type here)", name.c_str());
                        }
                    }, val);
            }
            ImGui::TreePop();
        }
    }
    
    const std::string& Material::GetVertexPath() const
    {
        static const std::string empty;
        return m_Res ? m_Res->VertexPath : empty;
    }
    
    const std::string& Material::GetFragmentPath() const
    {
        static const std::string empty;
        return m_Res ? m_Res->FragmentPath : empty;
    }
    
    void Material::LoadIntoShaderEditor() const
    {
        if (!m_Res || m_Res->VertexPath.empty() || m_Res->FragmentPath.empty())
            return;
        
        // Use the shader editor interface if available
        auto* editor = GetShaderEditorInterface();
        if (editor)
        {
            editor->LoadShaderFiles(m_Res->VertexPath, m_Res->FragmentPath);
        }
    }
}
