#pragma once

#include <glad/glad.h>

namespace Core::Renderer::Utils {

	const char* GLDebugSourceToString(GLenum source);
	const char* GLDebugTypeToString(GLenum type);
	const char* GLDebugSeverityToString(GLenum severity);

	void InitOpenGLDebugMessageCallback();

}