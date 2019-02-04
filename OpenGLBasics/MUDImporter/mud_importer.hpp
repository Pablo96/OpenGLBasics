#pragma once
#include <vector>

namespace MUDLoader
{
	#define MAX_VERTEX_BONES 4	// Max number of bones per vertex
#ifdef MUD_IMPORTER_HIGHP
	typedef double decimal;
#define PRECISION 20
#else
	typedef float decimal;
	#define PRECISION 10
#endif


	struct mat4
	{	
		decimal xx, yx, zx, wx;
		decimal xy, yy, zy, wy;
		decimal xz, yz, zz, wz;
		decimal xw, yw, zw, ww;
	};

	struct vec2
	{
		decimal x, y;
	};

	struct vec3
	{
		decimal x, y, z;
	};

	struct vec4
	{
		decimal x, y, z, w;
	};

	struct vec4u
	{
		unsigned int x, y, z, w;
	};
	
	typedef vec4 quatd;

	struct Bone
	{
		int id;
		Bone* parent;
		const char* debugName;
		mat4 offsetMatrix;
		std::vector<Bone*> children;
	};

	//###################################################################
	//				VERTEX, MESH AND MODEL
	//###################################################################

	struct Vertex
	{
		vec3 pos;
		vec3 normal;
		vec2 uvCoord = { 0 };
		vec4u indices = { 0 }; //Bones indices. Up to 4 bones
		vec4 weights = { 0 };	//Bones weight. 1 per bone (4 total)
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	};

	struct Model
	{
		std::vector<Mesh> meshes;
		Bone* skeleton = nullptr;
		std::vector<mat4*> bonesTransformsArray;
	};

	void LoadASCII(const char* filePath, Model** model);

	void LoadBinary(const char* filePath, Model** model);
}