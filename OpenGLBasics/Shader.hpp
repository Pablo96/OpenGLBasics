#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <GLM/glm.hpp>

typedef unsigned int uint32;

static std::string getShaderSrc(const char* fileName)
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

static void shaderCompileStatus(uint32 shader)
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

enum SHADER_TYPE
{
	VERTEX = GL_VERTEX_SHADER,
	FRAGMENT = GL_FRAGMENT_SHADER,
};

class Shader
{
	uint32 m_shader;
public:
	Shader(const char* in_path, const SHADER_TYPE in_type)
	{
		// Get the shader sources and compile them
		auto stringSource = getShaderSrc(in_path);
		auto shaderSource = stringSource.c_str();
		m_shader = glCreateShader(in_type);
		glShaderSource(m_shader, 1, &shaderSource, NULL);
		glCompileShader(m_shader);
		shaderCompileStatus(m_shader);
	}

	~Shader()
	{
		glDeleteShader(m_shader);
	}

	uint32 Get()
	{
		return m_shader;
	}
};

class Program
{
	// the program ID
	uint32 m_program;
public:

	// constructor reads and builds the shader
	Program(std::vector<Shader*>& in_shaders)
	{
		// create the shader program
		m_program = glCreateProgram();
		
		// attach the shaders to the program
		for (Shader* shader : in_shaders)
		{
			uint32 shaderID = shader->Get();
			glAttachShader(m_program, shaderID);
		}

		// link the atteched shaders to the program
		glLinkProgram(m_program);

		// check for linking errors
		int  success;
		char infoLog[512];
		glGetProgramiv(m_program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(m_program, 512, 0, infoLog);
			std::cout << "ERROR: Shader program linking failed.\n" << infoLog << "\n";
		}

		// detach the shaders to the program
		for (auto shader : in_shaders)
			glDetachShader(m_program, shader->Get());
	}

	// use/activate the shader
	void bind()
	{
		glUseProgram(m_program);
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
		return glGetUniformLocation(m_program, name);
	}
};

