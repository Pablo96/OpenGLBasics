#pragma once
#include "main.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <GLM/gtx/quaternion.hpp>
#include <GLM/gtx/matrix_interpolation.hpp>
#include <vector>
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE 
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE 
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

static tinygltf::TinyGLTF loader;

//###################################################################
//				BONE, POSE AND ANIMATION
//###################################################################
#define MAX_VERTEX_BONES 4	// Max number of bones per vertex

struct Bone
{
	uint32 id;
	glm::mat4 offsetMatrix;
	std::vector<Bone> children;
};

// TODO: add scaling
// Note: this are relative to parent bone.
struct BoneTransform
{
	glm::vec3 position;
	glm::quat rotation;
};

struct Pose
{
	float timeStamp; // time is played (in seconds)
	std::vector<BoneTransform> transforms;
};

struct Animation
{
	float duration = 0; // duration in ticks
	float ticksPerSec = 0;
	float currentTime = 0; // current time in seconds
	std::vector<Pose> poses;
};

void searchPoses(const Animation& anim, Pose& pose1, Pose& pose2, const float currentTime);
glm::mat4 interpolatePoseTransform(const BoneTransform& trans1, const BoneTransform& trans2, const float factor);

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
		glGenBuffers(1, &IBO);
		glGenBuffers(1, &WBO);

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
		// vertex tangent coords
		glEnableVertexAttribArray(3);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvCoord));

        // Transform attribute
        /*
        Since glVertexAttribPointer works on the binded buffer we most not forget to bind the buffer
        */
        glBindBuffer(GL_ARRAY_BUFFER, TBO);

		//##################################################
		//			TRANSFORM MATRIX
		//##################################################
        // Enable the vertex attributes
        int attribLocation = 4;
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

		//##################################################
		//			SKINNING INFO
		//##################################################
		attribLocation += 4;
		glEnableVertexAttribArray(attribLocation);
		glBindBuffer(GL_ARRAY_BUFFER, IBO);
		// Here we use 'glVertexAttribIPointer' so it stores Integer and do not convert it to float
		glVertexAttribIPointer(attribLocation, 4, GL_UNSIGNED_INT, 4 * sizeof(uint32) * 4, (void*)0);
		attribLocation += 1;
		glEnableVertexAttribArray(attribLocation);
		glBindBuffer(GL_ARRAY_BUFFER, WBO);
		glVertexAttribPointer(attribLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float) * 4, (void*)0);
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
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Material>* materials;
	
	// Skeleton AKA bone hierarchy of the mesh
	Bone* skeleton;

	// Animations
	std::vector<Animation> animations;

	// Animated transforms
	std::vector<glm::mat4> animatedTransforms;

	// DEBUG NAME
	const std::string name;
public:
    Model(const std::string& path, std::vector<Material>* inMaterials = nullptr, const std::string& inName="mesh")
        : materials(inMaterials), name(inName)
    {
        loadModel(path);

		animatedTransforms.emplace_back(glm::rotate(45.0f, glm::vec3(1.0, 0, 0)));

		for (uint32 i = 1; i < 50; i++)
		{
			animatedTransforms.emplace_back(glm::mat4(1.0f));
		}
	}

	void draw(Shader& shader, const uint32 count, const float deltaTime)
    {
		// animate skeleton
		if (animations.size() > 0)
			animate(deltaTime);

		// send animated transforms
		shader.setMat4fVec("bones", animatedTransforms);

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
private:
    void loadModel(const std::string& path)
    {
		tinygltf::Model model;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path, NULL);
		
		if (!warn.empty()) {
			printf("Warn: %s\n", warn.c_str());
		}

		if (!err.empty()) {
			printf("Err: %s\n", err.c_str());
		}

		if (!ret) {
			printf("Failed to parse glTF\n");
			exit(-1);
		}

		for (size_t i = 0; i < model.bufferViews.size(); i++) {
			const tinygltf::BufferView &bufferView = model.bufferViews[i];
			if (bufferView.target == 0) {
				std::cout << "WARN: bufferView.target is zero" << std::endl;
				continue;  // Unsupported bufferView.
			}

			const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

			
		}
    }
	

	void animate(const float deltaTime)
	{
		Animation animation = animations[0];
			
		float durationSec = animation.duration / animation.ticksPerSec;
		animation.currentTime += deltaTime;

		if (animation.currentTime > durationSec)
			animation.currentTime = animation.currentTime - durationSec;

		Pose pose1, pose2;
		searchPoses(animation, pose1, pose2, deltaTime);

		// interpolation factor between 0.0f  and 1.0f
		float factor = (pose2.timeStamp - pose1.timeStamp) / deltaTime;

		glm::mat4 transform;

		transform = interpolatePoseTransform(pose1.transforms[skeleton->id], pose2.transforms[skeleton->id], factor);
			
		animatedTransforms.emplace_back(transform);

		for (uint32 j = 0; j < skeleton->children.size(); j++)
		{
			applyAnimation(&skeleton->children[j], transform, pose1, pose2, factor);
		}
	}

	void applyAnimation(Bone* bone, const glm::mat4& parentTransform,
						const Pose& pose1, const Pose& pose2, const float factor)
	{
		glm::mat4 transform;

		transform = interpolatePoseTransform(pose1.transforms[bone->id], pose2.transforms[bone->id], factor);

		animatedTransforms.emplace_back(transform);

		for (uint32 j = 0; j < skeleton->children.size(); j++)
		{
			applyAnimation(&skeleton->children[j], parentTransform * transform, pose1, pose2, factor);
		}
	}
};


void searchPoses(const Animation& anim, Pose& pose1, Pose& pose2, const float currentTime)
{
	pose1 = anim.poses[0];
	for (size_t i = 1; i < anim.poses.size(); i++)
	{
		if (anim.poses[i].timeStamp > currentTime)
		{
			pose2 = anim.poses[i];
			pose1 = anim.poses[i - 1];
			return;
		}
	}
}

glm::mat4 interpolatePoseTransform(const BoneTransform& trans1, const BoneTransform& trans2, const float factor)
{
	glm::quat slerpQuat = glm::slerp(trans1.rotation, trans2.rotation, factor);
	glm::mat4 rotMat = glm::toMat4(slerpQuat);

	glm::vec3 lerpVec = glm::mix(trans1.position, trans2.position, factor);
	glm::mat4 posMat = glm::translate(lerpVec);

	return posMat * rotMat;
}

