#include "Renderer.h"

#include "GLUtils.h"

#include "Core/Debug/Profiler.h"

#include <iostream>
#include <print>

#include "stb_image.h"


namespace Core::Renderer {

	Texture CreateTexture(int width, int height)
	{
		PROFILE_FUNC();

		Texture result;
		result.Width = width;
		result.Height = height;

		glCreateTextures(GL_TEXTURE_2D, 1, &result.Handle);

		glTextureStorage2D(result.Handle, 1, GL_RGBA32F, width, height);

		glTextureParameteri(result.Handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(result.Handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(result.Handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(result.Handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return result;
	}

	Texture LoadTexture(const std::filesystem::path& path)
	{
		PROFILE_FUNC();

		int width, height, channels;
		std::string filepath = path.string();
		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

		if (!data)
		{
			std::cerr << "Failed to load texture: " << filepath << "\n";
			return {};
		}

		GLenum format = channels == 4 ? GL_RGBA :
			channels == 3 ? GL_RGB :
			channels == 1 ? GL_RED : 0;

		Texture result;
		result.Width = width;
		result.Height = height;

		glCreateTextures(GL_TEXTURE_2D, 1, &result.Handle);

		glTextureStorage2D(result.Handle, 1, (format == GL_RGBA ? GL_RGBA8 : GL_RGB8), width, height);

		glTextureSubImage2D(result.Handle, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

		glTextureParameteri(result.Handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(result.Handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(result.Handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(result.Handle, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenerateTextureMipmap(result.Handle);
		stbi_image_free(data);

		return result;
	}

	void BeginFrame(int w, int h)
	{
		PROFILE_FUNC();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

}
