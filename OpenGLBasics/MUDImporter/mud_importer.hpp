#pragma once
#include <vector>
#include <GLM/glm.hpp>

namespace MUDLoader
{

	typedef glm::mat4 mat4;
	typedef glm::vec2 vec2;
	typedef glm::vec3 vec3;
	typedef glm::vec4 vec4;
	typedef glm::uvec4 vec4u;
	typedef float decimal;
	
	//###################################################################
	//				VERTEX, MESH AND MODEL
	//###################################################################

	struct Vertex
	{
		vec3 pos;
		vec3 normal;
		vec2 uvCoord = vec2(0);
		vec4u indices = vec4u(0); //Bones indices. Up to 4 bones
		vec4 weights = vec4(0);	//Bones weight. 1 per bone (4 total)
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	};

	struct Model
	{
		std::vector<Mesh> meshes;
	};

	void LoadASCII(const char* filePath, Model** model);

	void LoadBinary(const char* filePath, Model** model);
}