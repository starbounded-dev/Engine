#pragma once

#include <glad/glad.h>
#include <cstdint>
#include <unordered_map>
#include <string>

namespace Core::Renderer {

	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, uint32_t bindingPoint);
		~UniformBuffer();

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;

		UniformBuffer(UniformBuffer&& other) noexcept;
		UniformBuffer& operator=(UniformBuffer&& other) noexcept;

		void SetData(const void* data, uint32_t size, uint32_t offset = 0);
		void Bind() const;
		void Unbind() const;

		inline uint32_t GetHandle() const { return m_Handle; }
		inline uint32_t GetBindingPoint() const { return m_BindingPoint; }
		inline uint32_t GetSize() const { return m_Size; }

		// Bind to a shader's uniform block
		static void BindUniformBlock(uint32_t shaderProgram, const std::string& blockName, uint32_t bindingPoint);

	private:
		uint32_t m_Handle = 0;
		uint32_t m_Size = 0;
		uint32_t m_BindingPoint = 0;
	};

}
