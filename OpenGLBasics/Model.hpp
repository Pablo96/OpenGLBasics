#pragma once
#include "main.h"
#include "Shader.hpp"
#include <mud_file/mud_importer.hpp>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLM/glm.hpp>
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

    void draw()
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (uint32)indices.size(), GL_UNSIGNED_INT, 0);
    }
};

class Model
{
    std::vector<Mesh*> meshes;
    std::string directory;
    std::vector<Material>* materials;
public:
    Model(const std::string& path, std::vector<Material>* inMaterials = nullptr)
        : materials(inMaterials)
    {
        loadModel(path);
    }

    void draw(Program& shader)
    {
        if (materials && (*materials).size() > 0)
            for (uint32 i = 0; i < meshes.size(); i++)
            {
                if ((*materials)[i].diffuse)
                    (*materials)[i].diffuse->bind(0);
                if ((*materials)[i].specular)
                    (*materials)[i].specular->bind(1);
                shader.setFloat("material.shininess", (*materials)[i].shininess);
                meshes[i]->draw();
            }
        else
            for (uint32 i = 0; i < meshes.size(); i++)
            {
                meshes[i]->draw();
            }
    }
private:
    void loadModel(const std::string& path)
    {
		MUDLoader::Model* out_model = nullptr;
		MUDLoader::LoadASCII(path.c_str(), &out_model);

		// To prevent exceptions later we return if we didnt load the model
		if (!out_model)
			return;

		for (auto mesh : out_model->meshes)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32> indices;

			for (auto vertex : mesh.vertices)
			{
				Vertex lt_vertex;

				lt_vertex.pos = vertex.pos;
				lt_vertex.normal = vertex.normal;
				lt_vertex.uvCoord = vertex.uvCoord;

				vertices.emplace_back(lt_vertex);
			}

			for (auto index : mesh.indices)
			{
				indices.emplace_back(index);
			}


			meshes.emplace_back(new Mesh(vertices, indices));
		}

		delete out_model;
		out_model = nullptr;
    }
};
