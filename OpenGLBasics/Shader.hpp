#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <GLM/glm.hpp>

typedef unsigned int uint32;

std::string getShaderSrc(const char* fileName)
{
	std::ifstream file;
	file.open(fileName);

	std::string output;
	std::string line;

	if (file.is_open())
	{
		while (file.good())
		{
			getline(file, line);
			output.append(line + "\n");
		}
	}
	else
	{
		std::cerr << "Unable to load shader: " << fileName << std::endl;
	}

	return output;
}

void shaderCompileStatus(uint32 shader)
{
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, 0, infoLog);
		std::cout << "ERROR: Shader compilation failed.\n" << infoLog << "\n";
	}
}

class Shader
{
public:
	// the program ID
	unsigned int ID;

	// constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath)
	{
		// Get the shader sources and compile them
		auto stringSource = getShaderSrc(vertexPath);
		auto vertexShaderSource = stringSource.c_str();
		uint32 vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, 0);
		glCompileShader(vertexShader);
		shaderCompileStatus(vertexShader);

		stringSource = getShaderSrc(fragmentPath);
		const char* fragmentShaderSource = stringSource.c_str();
		uint32 fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, 0);
		glCompileShader(fragmentShader);
		shaderCompileStatus(fragmentShader);

		// create the shader program
		ID = glCreateProgram();
		// attach the shaders to the program
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		// link the atteched shaders to the program
		glLinkProgram(ID);

		// check for linking errors
		int  success;
		char infoLog[512];
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, 0, infoLog);
			std::cout << "ERROR: Shader program linking failed.\n" << infoLog << "\n";
		}

		// if we dont detach them they wont be deleted until 
		// no program shader is using them
		glDetachShader(ID, vertexShader);
		glDetachShader(ID, fragmentShader);
		// if we dont use them in other shader program
		// we dont need the shaders once we've linked them
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	// use/activate the shader
	void bind()
	{
		glUseProgram(ID);
	}

	// utility uniform functions
	void setFloat(const char* name, const float value) const
	{
		glUniform1f(getLocation(name), value);
	}
	void setInt(const char* name, const int value) const
	{
		glUniform1i(getLocation(name), value);
	}
	void setVec4f(const char* name, const float x = 1.0f, const float y = 1.0f, const float z = 1.0f, const float w = 1.0f) const
	{
		glUniform4f(getLocation(name), x, y, z, w);
	}
	void setMat3f(const char* name, const glm::mat3& matrix) const
	{
		glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, &matrix[0][0]);
	}
	void setMat4f(const char* name, const glm::mat4& matrix) const
	{
		glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, &matrix[0][0]);
	}

private:
	int getLocation(const char* name) const
	{
		return glGetUniformLocation(ID, name);
	}
};

