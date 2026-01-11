#pragma once

#include "Renderer.h"
#include "Shader.h"
#include "UniformBuffer.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace Core::Renderer {

	class Material
	{
	public:
		Material();
		explicit Material(uint32_t shaderProgram);
		Material(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
		~Material() = default;

		Material(const Material&) = default;
		Material& operator=(const Material&) = default;

		Material(Material&&) = default;
		Material& operator=(Material&&) = default;

		// Shader
		void SetShader(uint32_t shaderProgram);
		uint32_t GetShader() const { return m_ShaderProgram; }

		// Texture slots
		void SetTexture(const std::string& name, const ::Renderer::Texture& texture, uint32_t slot = 0);
		void SetTexture(const std::string& name, GLuint textureHandle, uint32_t slot = 0);
		const ::Renderer::Texture* GetTexture(const std::string& name) const;

		// Uniform setters
		void SetFloat(const std::string& name, float value);
		void SetFloat2(const std::string& name, const glm::vec2& value);
		void SetFloat3(const std::string& name, const glm::vec3& value);
		void SetFloat4(const std::string& name, const glm::vec4& value);
		void SetMat3(const std::string& name, const glm::mat3& value);
		void SetMat4(const std::string& name, const glm::mat4& value);
		void SetInt(const std::string& name, int value);
		void SetInt2(const std::string& name, const glm::ivec2& value);
		void SetInt3(const std::string& name, const glm::ivec3& value);
		void SetInt4(const std::string& name, const glm::ivec4& value);
		void SetBool(const std::string& name, bool value);

		// Uniform Buffer Objects
		void SetUniformBuffer(const std::string& blockName, std::shared_ptr<UniformBuffer> ubo);
		std::shared_ptr<UniformBuffer> GetUniformBuffer(const std::string& blockName) const;

		// Bind material for rendering
		void Bind() const;

		// Clear all textures and uniforms
		void Clear();

	private:
		int32_t GetUniformLocation(const std::string& name) const;
		int32_t CacheUniformLocation(const std::string& name) const;

		uint32_t m_ShaderProgram = 0;
		mutable std::unordered_map<std::string, int32_t> m_UniformLocationCache;
		std::unordered_map<std::string, ::Renderer::Texture> m_Textures;
		std::unordered_map<std::string, uint32_t> m_TextureSlots; // Texture name -> slot
		std::unordered_map<std::string, std::shared_ptr<UniformBuffer>> m_UniformBuffers;
	};

}
