#pragma once
#include "main.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <GLM/gtx/quaternion.hpp>
#include <GLM/gtx/matrix_interpolation.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <vector>
#include <utility> // pair
#include "MUDImporter/mud_importer.hpp"
#include "Animator.h"


//###################################################################
//				VERTEX, MESH AND MODEL
//###################################################################

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uvCoord;
	glm::uvec4 indices = glm::vec4(0, 0, 0, 0); //Bones indices. Up to 4 bones
	glm::vec4 weights  = glm::vec4(0, 0, 0, 0);	//Bones weight. 1 per bone (4 total)
};

class Mesh
{
    uint32 VAO; // Vertex Array Object
    uint32 VBO; // Vertex Buffer Object
    uint32 EBO; // Elements Buffer Object

    uint32 TBO; // Transforms Buffer Object
    uint32 MBO; // Models Buffer Object

	uint32 IBO;
	uint32 WBO;

    bool m_init;
public:
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
public:
    Mesh(const std::vector<Vertex>& nVertices, const std::vector<uint32>& nIndices)
        : vertices(nVertices), indices(nIndices), m_init(false)
    {
        // Generate the buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenBuffers(1, &TBO);
        glGenBuffers(1, &MBO);

        // Bind the Array Object
        glBindVertexArray(VAO);



        // Bind Vertex Buffer to the Array Object
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Reserve memory and Send data to the Vertex Buffer
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


        // Bind Element Buffer to the Array Object
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // Reserve memory and Send data to the Elment Buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            &indices[0], GL_STATIC_DRAW);



        // Set the atribute pointers
        // vertex positions
        glEnableVertexAttribArray(0);
        // vertex normals
        glEnableVertexAttribArray(1);
        // vertex texture coords
        glEnableVertexAttribArray(2);
		// vertex bone weights
		glEnableVertexAttribArray(3);
		// vertex bone index
		glEnableVertexAttribArray(4);
        
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvCoord));
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
		// Here we use 'glVertexAttribIPointer' so it stores Integer and do not convert it to float
		glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, indices));

        // Transform attribute
        /*
        Since glVertexAttribPointer works on the binded buffer we most not forget to bind the buffer
        */
        glBindBuffer(GL_ARRAY_BUFFER, TBO);

		//##################################################
		//			TRANSFORM MATRIX
		//##################################################
        // Enable the vertex attributes
        int attribLocation = 5;
        glEnableVertexAttribArray(attribLocation + 0);
        glEnableVertexAttribArray(attribLocation + 1);
        glEnableVertexAttribArray(attribLocation + 2);
        glEnableVertexAttribArray(attribLocation + 3);
        // Set the buffer layout
        glVertexAttribPointer(attribLocation + 0, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)0);
        glVertexAttribPointer(attribLocation + 1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)(sizeof(float) * 4));
        glVertexAttribPointer(attribLocation + 2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)(sizeof(float) * 4 * 2));
        glVertexAttribPointer(attribLocation + 3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)(sizeof(float) * 4 * 3));
        // Set the attributes per instance
        glVertexAttribDivisor(attribLocation + 0, 1);
        glVertexAttribDivisor(attribLocation + 1, 1);
        glVertexAttribDivisor(attribLocation + 2, 1);
        glVertexAttribDivisor(attribLocation + 3, 1);

		//##################################################
		//			MODEL MATRIX
		//##################################################
        attribLocation += 4;
        glEnableVertexAttribArray(attribLocation + 0);
        glEnableVertexAttribArray(attribLocation + 1);
        glEnableVertexAttribArray(attribLocation + 2);
        glEnableVertexAttribArray(attribLocation + 3);
        glBindBuffer(GL_ARRAY_BUFFER, MBO);
        glVertexAttribPointer(attribLocation + 0, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)0);
        glVertexAttribPointer(attribLocation + 1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)(sizeof(float) * 4));
        glVertexAttribPointer(attribLocation + 2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)(sizeof(float) * 4 * 2));
        glVertexAttribPointer(attribLocation + 3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float) * 4, (void*)(sizeof(float) * 4 * 3));
        // Set the attributes per instance
        glVertexAttribDivisor(attribLocation + 0, 1);
        glVertexAttribDivisor(attribLocation + 1, 1);
        glVertexAttribDivisor(attribLocation + 2, 1);
        glVertexAttribDivisor(attribLocation + 3, 1);

    }

    void draw(const uint32 count) const
    {
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, (uint32)indices.size(), GL_UNSIGNED_INT, NULL, count);
    }

    void setTransforms(const uint32 count, const glm::mat4* matrices, const unsigned char type)
    {
        switch (type)
        {
        case 0:
            glBindBuffer(GL_ARRAY_BUFFER, TBO);
            break;
        case 1:
            glBindBuffer(GL_ARRAY_BUFFER, MBO);
            break;
        default: return;
        }



        int currentSize;
        int size = count * 16 * sizeof(float);
        // get the binded buffer size
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentSize);
        if (currentSize < size)
            glBufferData(GL_ARRAY_BUFFER, size, matrices, GL_DYNAMIC_DRAW);
        else
            glBufferSubData(GL_ARRAY_BUFFER, 0,  size, matrices);
    }
};

class Model
{
	bool init = false;

    std::string directory;
    std::vector<Mesh> meshes;
    std::vector<Material>* materials;
	
	// Skeleton AKA bone hierarchy of the mesh
	Bone* skeleton;

	// matrices of rest position in order
	std::vector<MUDLoader::tuple<int, glm::mat4*, glm::mat4*>> bindPoses;

	// Animations
	std::vector<Animation> animations;

	// DEBUG
	const std::string name;
	std::vector<glm::mat4> currentPose;

public:
    Model(const std::string& path, std::vector<Material>* inMaterials = nullptr, const std::string& inName="mesh")
        : materials(inMaterials), name(inName)
    {
		loadModel(path);

		currentPose.reserve(bindPoses.size());

		// DEBUG
		Pose pose;
		pose.timeStamp = 0;
		
		for (auto pair : bindPoses)
		{
			currentPose.emplace_back(*pair.first);
			
			PoseKey key;
			key.position = glm::vec3((*pair.first)[3]);
			key.rotation = glm::toQuat(*pair.first);
			
			pose.transforms.emplace_back(key);
		}

		Animation anim;
		anim.duration = 1;
		anim.ticksPerSec = 24;
		anim.poses.emplace_back(pose);

		animations.emplace_back(anim);
	}

	void draw(Shader& shader, const uint32 count, const float deltaTime)
    {
		if (!init)
		{
			init = !init;
			shader.setMat4f("bones", glm::mat4(1));
		}

		if (skeleton)
		{
			animate(deltaTime, animations[0], bindPoses, currentPose);
			shader.setMat4fVec("bones", currentPose);
		}

		// set materials
        if (materials && materials->size() > 0)
            for (uint32 i = 0; i < meshes.size(); i++)
            {
				shader.setVec4f("diffuseColor", (*materials)[i].diffuseCol.x, (*materials)[i].diffuseCol.y, (*materials)[i].diffuseCol.z);
                
				if ((*materials)[i].diffuse)
                    (*materials)[i].diffuse->bind(0);
				
				if ((*materials)[i].specular)
                    (*materials)[i].specular->bind(1);
				
                meshes[i].draw(count);
            }
        else
            for (uint32 i = 0; i < meshes.size(); i++)
            {
                meshes[i].draw(count);
            }
    }

    void setTransforms(const uint32 count, const glm::mat4* matrices, const unsigned char type)
    {
        for (auto mesh : meshes)
        {
            mesh.setTransforms(count, matrices, type);
        }
    }

    Mesh& getMesh(const uint32 index)
    {
        return meshes[index];
    }

	~Model()
	{
		if (skeleton)
			deleteBones(skeleton);
	}

private:
    void loadModel(const std::string& path)
    {
		MUDLoader::Model* model = nullptr;
		MUDLoader::LoadASCII(path.c_str(), &model);
		
		if (!model) 
			return;

		// Meshes of the model
		for (auto mesh : model->meshes)
		{
			std::vector<Vertex> vertices;
			for (auto vertex : mesh.vertices)
			{
				Vertex vert;

				vert.pos.x = vertex.pos.x;
				vert.pos.y = vertex.pos.y;
				vert.pos.z = vertex.pos.z;

				vert.normal.x = vertex.normal.x;
				vert.normal.y = vertex.normal.y;
				vert.normal.z = vertex.normal.z;

				vert.uvCoord.x = vertex.uvCoord.x;
				vert.uvCoord.y = vertex.uvCoord.y;

				vert.indices.x = vertex.indices.x;
				vert.indices.y = vertex.indices.y;
				vert.indices.z = vertex.indices.z;
				vert.indices.w = vertex.indices.w;

				vert.weights.x = vertex.weights.x;
				vert.weights.y = vertex.weights.y;
				vert.weights.z = vertex.weights.z;
				vert.weights.w = vertex.weights.w;

				vertices.emplace_back(vert);
			}

			meshes.emplace_back(Mesh(vertices, mesh.indices));
		}

		// Skeleton of the model
		skeleton = (Bone*) model->skeleton;
		
		if (skeleton)
		{
			int i = 0;
			// Get ordered array of pair (bind transforms, inverse bind transform)
			for (auto pairT : model->bindTransforms)
			{
				MUDLoader::tuple<int, glm::mat4*, glm::mat4*> pair;

				pair.parentID = pairT.parentID;
				pair.first = (glm::mat4*) pairT.first;
				pair.second= (glm::mat4*) pairT.second;
				
				bindPoses.emplace_back(pair);
			}
		}
		
		delete model;
    }

	void deleteBones(Bone* bone)
	{
		for (size_t i = 0; i < bone->children.size(); i++)
		{
			deleteBones(bone->children[i]);
		}
		delete bone;
		bone = nullptr;
	}
};


