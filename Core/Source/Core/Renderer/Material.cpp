#include "Material.h"

#include "Core/Debug/Profiler.h"

#include <iostream>

namespace Core::Renderer {

	Material::Material()
		: m_ShaderProgram(0)
	{
	}

	Material::Material(uint32_t shaderProgram)
		: m_ShaderProgram(shaderProgram)
	{
	}

	Material::Material(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
	{
		m_ShaderProgram = ::Renderer::CreateGraphicsShader(vertexPath, fragmentPath);
	}

	void Material::SetShader(uint32_t shaderProgram)
	{
		m_ShaderProgram = shaderProgram;
		m_UniformLocationCache.clear(); // Clear cache when shader changes
	}

	void Material::SetTexture(const std::string& name, const ::Renderer::Texture& texture, uint32_t slot)
	{
		m_Textures[name] = texture;
		m_TextureSlots[name] = slot;
	}

	void Material::SetTexture(const std::string& name, GLuint textureHandle, uint32_t slot)
	{
		::Renderer::Texture texture;
		texture.Handle = textureHandle;
		m_Textures[name] = texture;
		m_TextureSlots[name] = slot;
	}

	const ::Renderer::Texture* Material::GetTexture(const std::string& name) const
	{
		auto it = m_Textures.find(name);
		if (it != m_Textures.end())
			return &it->second;
		return nullptr;
	}

	void Material::SetFloat(const std::string& name, float value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform1f(m_ShaderProgram, location, value);
	}

	void Material::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform2f(m_ShaderProgram, location, value.x, value.y);
	}

	void Material::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform3f(m_ShaderProgram, location, value.x, value.y, value.z);
	}

	void Material::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform4f(m_ShaderProgram, location, value.x, value.y, value.z, value.w);
	}

	void Material::SetMat3(const std::string& name, const glm::mat3& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniformMatrix3fv(m_ShaderProgram, location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Material::SetMat4(const std::string& name, const glm::mat4& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniformMatrix4fv(m_ShaderProgram, location, 1, GL_FALSE, glm::value_ptr(value));
	}

	void Material::SetInt(const std::string& name, int value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform1i(m_ShaderProgram, location, value);
	}

	void Material::SetInt2(const std::string& name, const glm::ivec2& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform2i(m_ShaderProgram, location, value.x, value.y);
	}

	void Material::SetInt3(const std::string& name, const glm::ivec3& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform3i(m_ShaderProgram, location, value.x, value.y, value.z);
	}

	void Material::SetInt4(const std::string& name, const glm::ivec4& value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform4i(m_ShaderProgram, location, value.x, value.y, value.z, value.w);
	}

	void Material::SetBool(const std::string& name, bool value)
	{
		int32_t location = GetUniformLocation(name);
		if (location != -1)
			glProgramUniform1i(m_ShaderProgram, location, value ? 1 : 0);
	}

	void Material::SetUniformBuffer(const std::string& blockName, std::shared_ptr<UniformBuffer> ubo)
	{
		if (ubo)
		{
			m_UniformBuffers[blockName] = ubo;
			UniformBuffer::BindUniformBlock(m_ShaderProgram, blockName, ubo->GetBindingPoint());
		}
		else
		{
			m_UniformBuffers.erase(blockName);
		}
	}

	std::shared_ptr<UniformBuffer> Material::GetUniformBuffer(const std::string& blockName) const
	{
		auto it = m_UniformBuffers.find(blockName);
		if (it != m_UniformBuffers.end())
			return it->second;
		return nullptr;
	}

	void Material::Bind() const
	{
		PROFILE_FUNC();

		if (m_ShaderProgram == 0)
			return;

		glUseProgram(m_ShaderProgram);

		// Bind textures
		for (const auto& [name, texture] : m_Textures)
		{
			uint32_t slot = m_TextureSlots.at(name);
			glBindTextureUnit(slot, texture.Handle);

			// Set sampler uniform if it exists
			int32_t location = GetUniformLocation(name);
			if (location != -1)
				glProgramUniform1i(m_ShaderProgram, location, static_cast<int>(slot));
		}

		// Bind uniform buffers
		for (const auto& [name, ubo] : m_UniformBuffers)
		{
			ubo->Bind();
		}
	}

	void Material::Clear()
	{
		m_Textures.clear();
		m_TextureSlots.clear();
		m_UniformBuffers.clear();
		m_UniformLocationCache.clear();
	}

	int32_t Material::GetUniformLocation(const std::string& name) const
	{
		if (m_ShaderProgram == 0)
			return -1;

		// Check cache first
		auto it = m_UniformLocationCache.find(name);
		if (it != m_UniformLocationCache.end())
		{
			return it->second;
		}

		// Query OpenGL and cache
		return CacheUniformLocation(name);
	}

	int32_t Material::CacheUniformLocation(const std::string& name) const
	{
		int32_t location = glGetUniformLocation(m_ShaderProgram, name.c_str());
		m_UniformLocationCache[name] = location;

		if (location == -1)
		{
			// Optional: warn about missing uniforms (comment out for production)
			// std::cerr << "Material::GetUniformLocation: Uniform '" << name << "' not found in shader!" << std::endl;
		}

		return location;
	}

}
