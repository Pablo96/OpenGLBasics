#pragma once
#include <vector>

namespace MUDLoader
{
	#define MAX_VERTEX_BONES 4	// Max number of bones per vertex

	struct mat4
	{
		double xx, xy, xz, xw;
		double yx, yy, yz, yw;
		double zx, zy, zz, zw;
		double wx, wy, wz, ww;
	};

	template<typename T>
	struct vec2
	{
		T x, y;
	};

	template<typename T>
	struct vec3
	{
		T x, y, z;
	};

	template<typename T>
	struct vec4
	{
		T x, y, z, w;
	};

	typedef vec2<double> vec2d;
	typedef vec3<double> vec3d;
	typedef vec4<double> vec4d;
	typedef vec4<unsigned int> vec4u;
	typedef vec4d quatd;

	struct Bone
	{
		int id;
		const char* debugName;
		mat4 offsetMatrix;
		std::vector<Bone*> children;
	};

	//###################################################################
	//				VERTEX, MESH AND MODEL
	//###################################################################

	struct Vertex
	{
		vec3d pos;
		vec3d normal;
		vec2d uvCoord = { 0, 0 };
		vec4u indices = { 0, 0, 0, 0 }; //Bones indices. Up to 4 bones
		vec4d weights = { 0.0, 0.0, 0.0, 0.0 };	//Bones weight. 1 per bone (4 total)
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
		std::vector<mat4*> boneArray;
	};

	void LoadASCII(const char* filePath, Model** model);

	void LoadBinary(const char* filePath, Model** model);
}