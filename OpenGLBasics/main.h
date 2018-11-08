#pragma once
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


typedef unsigned int uint32;

class Texture
{
public:
	uint32 ID;

	Texture(const char* fileName)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(fileName, &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cout << "Failed to load texture" << std::endl;
			return;
		}

		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_2D, ID);

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// generate texture
		GLenum channels = (nrChannels == 4) ? GL_SRGB_ALPHA : GL_SRGB;
		GLenum channelsImage = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, channelsImage, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}

	void bind(const uint32 unit = 0) const
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, ID);
	}
};



struct Material
{
    Texture* diffuse;
    Texture* specular;
	Texture* normal;
    float shininess;
};



