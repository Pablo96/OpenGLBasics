#pragma once
#include "main.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <GLM/gtx/quaternion.hpp>
#include <GLM/gtx/matrix_interpolation.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <unordered_map>



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

		for (uint32 i = 0; i < 50; i++)
		{
			animatedTransforms.emplace_back(glm::mat4(1.0f));
		}
	}

	void draw(Shader& shader, const uint32 count, const float deltaTime)
    {
		/*
		// animate skeleton
		if (animations.size() > 0)
			animate(deltaTime);

		*/
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
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, 
			  aiProcess_FlipUVs
			| aiProcess_Triangulate
			| aiProcess_LimitBoneWeights
			| aiProcess_GenSmoothNormals
			| aiProcess_CalcTangentSpace
		);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);

		if (scene->HasAnimations())
			processAnimations(scene);
		
    }
    
	void processNode(aiNode *node, const aiScene *scene)
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }
    
	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
		
		// process vertex data
        for (uint32 i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // process vertex positions, normals and texture coordinates
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.pos = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
			
			// store tangents and bitangets
			if (mesh->HasTangentsAndBitangents())
			{

			}

			// store texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uvCoord = vec;
            }
            else
                vertex.uvCoord = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

		if (mesh->HasBones())
		{
			// process bones data
			for (uint32 i = 0; i < mesh->mNumBones; i++)
			{
				aiBone* bone = mesh->mBones[i];

				for (uint32 j = 0; j < MAX_VERTEX_BONES; j++)
				{
					uint32 vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
					float weight = mesh->mBones[i]->mWeights[j].mWeight;

					vertices[vertexID].weights[j] = weight;
					vertices[vertexID].indices[j] = i;
				}
			}
		}

        // process indices
        for (uint32 i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices);
    }
	
	void processAnimations(const aiScene *scene)
	{
		for (uint32 i = 0; i < scene->mNumAnimations; i++)
		{
			const aiAnimation* assimpAnim = scene->mAnimations[i];
			Animation animation;

			animation.duration = (float)assimpAnim->mDuration;
			animation.ticksPerSec = (float)assimpAnim->mTicksPerSecond;

			// numPoses is the higher num of transforms
			uint32 numPoses = 0;
			for (uint32 j = 0; j < assimpAnim->mNumChannels; j++)
			{
				const aiNodeAnim* channel = assimpAnim->mChannels[j];
				uint32 num =  (channel->mNumPositionKeys > channel->mNumRotationKeys)
						   ?  (channel->mNumPositionKeys > channel->mNumScalingKeys) ? channel->mNumPositionKeys : channel->mNumScalingKeys
						   :  (channel->mNumRotationKeys > channel->mNumScalingKeys) ? channel->mNumRotationKeys : channel->mNumScalingKeys;
				numPoses = (num > numPoses) ? num : numPoses;
			}
			animation.poses.resize(numPoses);

			// each channel is a bone with all its transformations
			for (uint32 j = 0; j < assimpAnim->mNumChannels; j++)
			{
				const aiNodeAnim* channel = assimpAnim->mChannels[j];
				

				for (uint32 k = 0; k < numPoses; k++)
				{
					glm::mat4 transform;
					if (channel->mNumScalingKeys >= k)
					{
						const auto scaleKey = channel->mScalingKeys[k].mValue;
						glm::vec3 scale;
						scale.x = scaleKey.x;
						scale.y = scaleKey.y;
						scale.z = scaleKey.z;

						transform = glm::scale(scale);
					}
					if (channel->mNumRotationKeys >= k)
					{
						const auto rotKey = channel->mRotationKeys[k].mValue;
						glm::quat quaternion;
						quaternion.x = rotKey.x;
						quaternion.y = rotKey.y;
						quaternion.z = rotKey.z;
						quaternion.w = rotKey.w;

						transform *= glm::toMat4(quaternion);
					}
					if (channel->mNumPositionKeys >= k)
					{
						const auto posKey= channel->mPositionKeys[k].mValue;
						glm::vec3 translation;
						translation.x = posKey.x;
						translation.y = posKey.y;
						translation.z = posKey.z;
						transform *= glm::translate(translation);
					}

					//animation.poses[k].transforms.emplace_back(transform);
				}
			}

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

