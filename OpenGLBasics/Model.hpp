#pragma once
#include "main.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>


struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uvCoord;
	glm::vec3 tangent;
};

class Mesh
{
    uint32 VAO; // Vertex Array Object
    uint32 VBO; // Vertex Buffer Object
    //uint32 IBO; // Per Instance attributes Buffer Object
    uint32 EBO; // Elements Buffer Object
public:
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
public:
    Mesh(const std::vector<Vertex>& nVertices, const std::vector<uint32>& nIndices)
        : vertices(nVertices), indices(nIndices)
    {
        // Generate the buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uvCoord));
    }

    void draw(Shader& shader)
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (uint32)indices.size(), GL_UNSIGNED_INT, 0);
    }
};

class Model
{
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Material>* materials;
public:
    Model(const std::string& path, std::vector<Material>* inMaterials = nullptr)
        : materials(inMaterials)
    {
        loadModel(path);
    }

    void draw(Shader& shader)
    {
        if (materials && (*materials).size() > 0)
            for (uint32 i = 0; i < meshes.size(); i++)
            {
                if ((*materials)[i].diffuse)
                    (*materials)[i].diffuse->bind(0);
				if ((*materials)[i].specular)
					(*materials)[i].specular->bind(1);
                shader.setFloat("material.intensity", (*materials)[i].intensity);
                meshes[i].draw(shader);
            }
        else
            for (uint32 i = 0; i < meshes.size(); i++)
            {
                meshes[i].draw(shader);
            }
    }
private:
    void loadModel(const std::string& path)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
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

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
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

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
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

        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices);
    }
};

class MeshInstanced
{
    uint32 VAO; // Vertex Array Object
    uint32 VBO; // Vertex Buffer Object
    uint32 EBO; // Elements Buffer Object

    uint32 TBO; // Transforms Buffer Object
    uint32 MBO; // Models Buffer Object
    uint32 NBO; // Normal mat Buffer Object
    bool m_init;
public:
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
public:
    MeshInstanced(const std::vector<Vertex>& nVertices, const std::vector<uint32>& nIndices)
        : vertices(nVertices), indices(nIndices), m_init(false)
    {
        // Generate the buffers
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenBuffers(1, &TBO);
        glGenBuffers(1, &MBO);
        glGenBuffers(1, &NBO);

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
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

        // Transform attribute
        /*
        Since glVertexAttribPointer works on the binded buffer we most not forget to bind the buffer
        */
        glBindBuffer(GL_ARRAY_BUFFER, TBO);

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

        attribLocation += 4;
        glEnableVertexAttribArray(attribLocation + 0);
        glEnableVertexAttribArray(attribLocation + 1);
        glEnableVertexAttribArray(attribLocation + 2);
        glBindBuffer(GL_ARRAY_BUFFER, NBO);
        glVertexAttribPointer(attribLocation + 0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float) * 4, (void*)0);
        glVertexAttribPointer(attribLocation + 1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float) * 4, (void*)(sizeof(float) * 4));
        glVertexAttribPointer(attribLocation + 2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float) * 4, (void*)(sizeof(float) * 4 * 2));
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

    void setTransforms(const uint32 count, const glm::mat3* matrices)
    {
        int currentSize;
        int size = count * 9 * sizeof(float);
        glBindBuffer(GL_ARRAY_BUFFER, NBO);
        
        // get the binded buffer size
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentSize);
        if (currentSize < size)
            glBufferData(GL_ARRAY_BUFFER, size, matrices, GL_DYNAMIC_DRAW);
        else
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, matrices);
    }
};

class ModelInstanced
{
    std::vector<MeshInstanced> meshes;
    std::string directory;
    std::vector<Material>* materials;
	const std::string name;
public:
    ModelInstanced(const std::string& path, std::vector<Material>* inMaterials = nullptr, const std::string& inName="mesh")
        : materials(inMaterials), name(inName)
    {
        loadModel(path);
    }

    void draw(Shader& shader, const uint32 count)
    {
        if (materials && materials->size() > 0)
            for (uint32 i = 0; i < meshes.size(); i++)
            {
                if ((*materials)[i].diffuse)
                    (*materials)[i].diffuse->bind(0);
				
				if ((*materials)[i].normal)
					(*materials)[i].normal->bind(2);

				shader.setFloat("material.intensity", (*materials)[i].intensity);
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

    void setTransforms(const uint32 count, const glm::mat3* matrices)
    {
        for (auto mesh : meshes)
        {
            mesh.setTransforms(count, matrices);
        }
    }

    MeshInstanced& getMesh(const uint32 index)
    {
        return meshes[index];
    }
private:
    void loadModel(const std::string& path)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, 
			  aiProcess_Triangulate
			| aiProcess_CalcTangentSpace
			| aiProcess_FlipUVs
			| aiProcess_GenSmoothNormals
		);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
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
    MeshInstanced processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
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

			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.tangent = vector;

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
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

        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return MeshInstanced(vertices, indices);
    }
};
