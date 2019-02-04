#include "tinyxml2.h"
#include "mud_importer.hpp"
#include <sstream>
#include <iostream>
#include <iterator>   // istream_iterator

void quaternionToMatrix(MUDLoader::quatd& q, MUDLoader::mat4& mat)
{
	mat.xx = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
	mat.xy = 2 * q.x * q.y - 2 * q.z * q.w;
	mat.xz = 2 * q.x * q.z + 2 * q.y * q.w;

	mat.yx = 2 * q.x * q.y + 2 * q.z * q.w;
	mat.yy = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
	mat.yz = 2 * q.y * q.z - 2 * q.x * q.w;

	mat.zx = 2 * q.x * q.z - 2 * q.y * q.w;
	mat.zy = 2 * q.y * q.z + 2 * q.x * q.w;
	mat.zz = 1 - 2 * q.x * q.x - 2 * q.y * q.y;

	mat.wx = 0;
	mat.wy = 0;
	mat.wz = 0;
	mat.ww = 1;
}

void translationToMatrix(MUDLoader::vec3& vec, MUDLoader::mat4& mat)
{
	mat.xw = vec.x;
	mat.yw = vec.y;
	mat.zw = vec.z;
}


void helperStrToVec3(const char* charArray, MUDLoader::vec3& vec)
{
	std::string string(charArray);
	
	size_t first = string.find_first_of(", ");

	auto sub = string.substr(0, PRECISION);

	// X
	{
		size_t first = sub.find_first_of(", ");
		if (first != std::string::npos)
			sub = sub.replace(first, std::string::npos, "");
		vec.x = (MUDLoader::decimal) atof(sub.c_str());
	}

	// Y
	{
		first = string.find(", ", first) + 2;
		sub = string.substr(first, PRECISION);

		size_t first = sub.find_first_of(", ");
		if (first != std::string::npos)
			sub = sub.replace(first, std::string::npos, "");

		vec.y = (MUDLoader::decimal) atof(sub.c_str());
	}

	// Z
	{
		first = string.find(", ", first) + 2;
		sub = string.substr(first, PRECISION);

		vec.z = (MUDLoader::decimal) atof(sub.c_str());
	}
}

void helperStrToVec4(const char* charArray, MUDLoader::vec4& vec)
{
	std::string string(charArray);

	size_t first = string.find_first_of(", ");

	auto sub = string.substr(0, PRECISION);

	// X
	{
		size_t first = sub.find_first_of(", ");
		if (first != std::string::npos)
			sub = sub.replace(first, std::string::npos, "");
		vec.x = (MUDLoader::decimal) atof(sub.c_str());
	}

	// Y
	{
		first = string.find(", ", first) + 2;
		sub = string.substr(first, PRECISION);
		
		size_t first = sub.find_first_of(", ");
		if (first != std::string::npos)
			sub = sub.replace(first, std::string::npos, "");
		
		vec.y = (MUDLoader::decimal) atof(sub.c_str());
	}

	// Z
	{
		first = string.find(", ", first) + 2;
		sub = string.substr(first, PRECISION);
		
		
		size_t first = sub.find_first_of(", ");
		if (first != std::string::npos)
			sub = sub.replace(first, std::string::npos, "");

		vec.z = (MUDLoader::decimal) atof(sub.c_str());
	}

	// W
	{
		first = string.find(", ", first) + 2;
		sub = string.substr(first, PRECISION);

		vec.w = (MUDLoader::decimal) atof(sub.c_str());
	}
}


void helperBoneBuild(MUDLoader::Bone& bone, tinyxml2::XMLElement* boneNode)
{
	auto translation = boneNode->FindAttribute("translation")->Value();
	auto rotation = boneNode->FindAttribute("rotation")->Value();
	auto name = boneNode->FindAttribute("name")->Value();
	auto id = boneNode->FindAttribute("id")->Value();
	bone.debugName = name;
	bone.id = atoi(id);

	MUDLoader::vec3 translationVec = MUDLoader::vec3();
	helperStrToVec3(translation, translationVec);

	MUDLoader::quatd rotationQuat = MUDLoader::quatd();
	helperStrToVec4(rotation, rotationQuat);

	quaternionToMatrix(rotationQuat, bone.offsetMatrix);
	translationToMatrix(translationVec, bone.offsetMatrix);
}

void skeletonBuild(tinyxml2::XMLElement* node, MUDLoader::Bone* parent, std::vector<MUDLoader::mat4*>& array)
{
	for (auto siblingNode = node; siblingNode != nullptr; siblingNode = siblingNode->NextSiblingElement("bone"))
	{
		MUDLoader::Bone* boneTmp = new MUDLoader::Bone();
		helperBoneBuild(*boneTmp, siblingNode);
		
		// double linking
		parent->children.emplace_back(boneTmp);
		boneTmp->parent = parent;

		// Matrix array
		array.emplace_back(&boneTmp->offsetMatrix);

		skeletonBuild(siblingNode->FirstChildElement("bone"), boneTmp, array);
	}
}

void MUDLoader::LoadASCII(const char * filePath, Model** model)
{
	tinyxml2::XMLDocument doc;
	auto loaded = doc.LoadFile(filePath);

	if (loaded == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND)
	{
		std::cout << "FILE NOT FOUND: " << filePath << std::endl;
		return;
	}

	tinyxml2::XMLElement* modelNode = doc.FirstChildElement("model");

	if (!modelNode)
	{
		std::cout << "FILE NOT A MUD MODEL: " << filePath << std::endl;
		return;
	}

	std::vector<Mesh> meshes;

	// Meshes of the model
	for (auto meshNode = modelNode->FirstChildElement("mesh");
		meshNode != nullptr;
		meshNode = meshNode->NextSiblingElement("mesh"))
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		// Mesh vertices
		for (auto vertexNode = meshNode->FirstChildElement("vertex");
			vertexNode != nullptr;
			vertexNode = vertexNode->NextSiblingElement("vertex"))
		{
			Vertex vertex;

			// Position
			auto text = vertexNode->FirstChildElement("position")->FirstAttribute()->Value();
			{
				std::stringstream string(text);
				string >> vertex.pos.x;
				string >> vertex.pos.y;
				string >> vertex.pos.z;
			}

			// Normal
			text = vertexNode->FirstChildElement("normal")->FirstAttribute()->Value();
			{
				std::stringstream string(text);
				string >> vertex.normal.x;
				string >> vertex.normal.y;
				string >> vertex.normal.z;
			}

			// Bone Indices
			auto hasIndices = vertexNode->FirstChildElement("indices");
			if (hasIndices)
			{
				text = vertexNode->FirstChildElement("indices")->FirstAttribute()->Value();
				{
					std::stringstream string(text);
					string >> vertex.indices.x;
					string >> vertex.indices.y;
					string >> vertex.indices.z;
					string >> vertex.indices.w;
				}
				// Bone Weights
				text = vertexNode->FirstChildElement("weights")->FirstAttribute()->Value();
				{
					std::stringstream string(text);
					string >> vertex.weights.x;
					string >> vertex.weights.y;
					string >> vertex.weights.z;
					string >> vertex.weights.w;
				}
			}
			vertices.emplace_back(vertex);
		}

		// Vertex indices
		tinyxml2::XMLElement* indicesNode = meshNode->FirstChildElement("indices");
		unsigned int count = std::strtoul(indicesNode->FirstAttribute()->Value(), nullptr, 0);
		auto text = indicesNode->FindAttribute("values")->Value();
		std::stringstream string(text);
		unsigned int index;
		for (size_t i = 0; i < count; i++)
		{
			string >> index;
			indices.emplace_back(index);
		}

		meshes.emplace_back(Mesh({ vertices, indices }));
	}

	// Skeleton of the model
	bool init = true;
	tinyxml2::XMLElement* skeletonNode = modelNode->FirstChildElement("skeleton");
	
	Bone* skeleton = nullptr;
	std::vector<mat4*> boneArray;

	if (skeletonNode)
	{
		// build the root bone (skeleton must have a unique root bone!)
		skeleton = new Bone();
		auto rootNode = skeletonNode->FirstChildElement("bone");
		helperBoneBuild(*skeleton, rootNode);
		
		// build skeleton and transform array (should be sorted)
		skeletonBuild(rootNode->FirstChildElement("bone"), skeleton, boneArray);
	}
	*model = new Model({ meshes, skeleton, boneArray });
}
