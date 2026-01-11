#include "Buffer.h"

#include "Core/Debug/Profiler.h"

#include <cstring>

namespace Core::Renderer {

	VertexBuffer::VertexBuffer(uint32_t size, BufferUsage usage)
		: m_Size(size), m_Usage(usage)
	{
		PROFILE_FUNC();

		glCreateBuffers(1, &m_Handle);
		glNamedBufferData(m_Handle, size, nullptr, static_cast<GLenum>(usage));
	}

	VertexBuffer::VertexBuffer(const void* data, uint32_t size, BufferUsage usage)
		: m_Size(size), m_Usage(usage)
	{
		PROFILE_FUNC();

		glCreateBuffers(1, &m_Handle);
		glNamedBufferData(m_Handle, size, data, static_cast<GLenum>(usage));
	}

	VertexBuffer::~VertexBuffer()
	{
		if (m_Handle != 0)
			glDeleteBuffers(1, &m_Handle);
	}

	VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
		: m_Handle(other.m_Handle), m_Size(other.m_Size), m_Usage(other.m_Usage), m_Layout(other.m_Layout)
	{
		other.m_Handle = 0;
		other.m_Size = 0;
	}

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Handle != 0)
				glDeleteBuffers(1, &m_Handle);

			m_Handle = other.m_Handle;
			m_Size = other.m_Size;
			m_Usage = other.m_Usage;
			m_Layout = other.m_Layout;

			other.m_Handle = 0;
			other.m_Size = 0;
		}

		return *this;
	}

	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_Handle);
	}

	void VertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		PROFILE_FUNC();

		if (offset + size > m_Size)
		{
			// Resize buffer if needed
			m_Size = offset + size;
			glNamedBufferData(m_Handle, m_Size, nullptr, static_cast<GLenum>(m_Usage));
		}

		glNamedBufferSubData(m_Handle, offset, size, data);
	}

	IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count, BufferUsage usage)
		: m_Count(count), m_Usage(usage)
	{
		PROFILE_FUNC();

		glCreateBuffers(1, &m_Handle);
		glNamedBufferData(m_Handle, count * sizeof(uint32_t), indices, static_cast<GLenum>(usage));
	}

	IndexBuffer::~IndexBuffer()
	{
		if (m_Handle != 0)
			glDeleteBuffers(1, &m_Handle);
	}

	IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
		: m_Handle(other.m_Handle), m_Count(other.m_Count), m_Usage(other.m_Usage)
	{
		other.m_Handle = 0;
		other.m_Count = 0;
	}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Handle != 0)
				glDeleteBuffers(1, &m_Handle);

			m_Handle = other.m_Handle;
			m_Count = other.m_Count;
			m_Usage = other.m_Usage;

			other.m_Handle = 0;
			other.m_Count = 0;
		}

		return *this;
	}

	void IndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Handle);
	}

	void IndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}
