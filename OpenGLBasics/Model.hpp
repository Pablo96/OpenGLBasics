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

class SubMesh
{
    uint32 EBO; // Elements Buffer Object
public:
    std::vector<uint32> indices;
	Material material;
public:
	SubMesh(const std::vector<uint32>& inIndices)
        : indices(inIndices)
    {
        // Generate the buffers
        glGenBuffers(1, &EBO);

        // Bind Element Buffer to the Array Object
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // Reserve memory and Send data to the Elment Buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            &indices[0], GL_STATIC_DRAW);
    }

    void draw(Program& shader)
    {
		if (material.diffuse)
			material.diffuse->bind(0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, (uint32)indices.size(), GL_UNSIGNED_INT, 0);
    }
};

class Mesh
{
	uint32 VAO; // Vertex Array Object
	uint32 VBO; // Vertex Buffer Object
    std::string directory;
    std::vector<Vertex> m_vertices;
    std::vector<SubMesh> m_submeshes;
public:
	Mesh(const std::string& path)
    {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

        loadModel(path);

		glGenBuffers(1, &VBO);
		// Bind Vertex Buffer to the Array Object
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// Reserve memory and Send data to the Vertex Buffer
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

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

    void draw(Program& shader)
    {
		glBindVertexArray(VAO);
		for (auto submesh : m_submeshes)
			submesh.draw(shader);
    }

	void setMaterials(std::vector<Material>& in_materials)
	{
		auto loop_end = (in_materials.size() < m_submeshes.size()) ? in_materials.size() : m_submeshes.size();
		uint32 i;
		for (i = 0; i < loop_end; i++)
			m_submeshes.at(i).material = in_materials.at(i);

		if (in_materials.size() > m_submeshes.size())
			return;

		auto last_mat = in_materials.at(--i);
		for (; i < m_submeshes.size(); i++)
			m_submeshes.at(i).material = last_mat;
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
				if (m_submeshes.size() > 0)
					index += (uint32) m_vertices.size();
				indices.emplace_back(index);
			}

			m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
			m_submeshes.emplace_back(SubMesh(indices));
		}
		delete out_model;
		out_model = nullptr;
    }
};
