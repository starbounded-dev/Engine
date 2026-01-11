#include "UniformBuffer.h"

#include "Core/Debug/Profiler.h"

#include <glad/glad.h>
#include <iostream>

namespace Core::Renderer {

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t bindingPoint)
		: m_Size(size), m_BindingPoint(bindingPoint)
	{
		PROFILE_FUNC();

		glCreateBuffers(1, &m_Handle);
		glNamedBufferData(m_Handle, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_Handle);
	}

	UniformBuffer::~UniformBuffer()
	{
		if (m_Handle != 0)
			glDeleteBuffers(1, &m_Handle);
	}

	UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
		: m_Handle(other.m_Handle), m_Size(other.m_Size), m_BindingPoint(other.m_BindingPoint)
	{
		other.m_Handle = 0;
		other.m_Size = 0;
		other.m_BindingPoint = 0;
	}

	UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Handle != 0)
				glDeleteBuffers(1, &m_Handle);

			m_Handle = other.m_Handle;
			m_Size = other.m_Size;
			m_BindingPoint = other.m_BindingPoint;

			other.m_Handle = 0;
			other.m_Size = 0;
			other.m_BindingPoint = 0;
		}

		return *this;
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		PROFILE_FUNC();

		if (offset + size > m_Size)
		{
			std::cerr << "UniformBuffer::SetData: Size exceeds buffer size!" << std::endl;
			return;
		}

		glNamedBufferSubData(m_Handle, offset, size, data);
	}

	void UniformBuffer::Bind() const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_Handle);
	}

	void UniformBuffer::Unbind() const
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, 0);
	}

	void UniformBuffer::BindUniformBlock(uint32_t shaderProgram, const std::string& blockName, uint32_t bindingPoint)
	{
		PROFILE_FUNC();

		GLuint blockIndex = glGetUniformBlockIndex(shaderProgram, blockName.c_str());
		if (blockIndex == GL_INVALID_INDEX)
		{
			std::cerr << "UniformBuffer::BindUniformBlock: Uniform block '" << blockName << "' not found in shader!" << std::endl;
			return;
		}

		glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);
	}

}
