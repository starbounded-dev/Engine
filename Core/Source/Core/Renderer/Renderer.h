#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>

namespace Core::Renderer {

	struct Texture
	{
		GLuint Handle = 0;
		uint32_t Width = 0;
		uint32_t Height = 0;
	};


	Texture CreateTexture(int width, int height);
	Texture LoadTexture(const std::filesystem::path& path);
	void BeginFrame(int w, int h);
}