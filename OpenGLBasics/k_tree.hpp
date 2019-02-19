#pragma once
#include <unordered_set>
#include <GLM/glm.hpp>

class Node
{
	friend class ktree;

private:
	glm::mat4 local_transform;
	glm::mat4 world_transform;

	const void* model;

	Node* parent;
	std::unordered_set<Node*> children;
protected:
	Node(const glm::mat4& in_transform, const void* in_model = nullptr, Node* in_parent = nullptr)
		: model(in_model), local_transform(in_transform), parent(in_parent)
	{
		if (parent)
		{
			parent->children.emplace(this);
		}
		else
		{
			world_transform = local_transform;
		}
	}
};

class ktree
{
	Node* root;

public:
	ktree(const glm::vec3* scene_position = nullptr, glm::mat4* scene_rotation = nullptr)
	{
		glm::mat4 global_center;

		// Location
		global_center = (scene_position) ? glm::translate(*scene_position) : glm::mat4(1.0f);

		// Rotaiton
		if (scene_rotation)
			global_center = global_center * (*scene_rotation);
		
		// Root node alloc
		root = new Node(global_center);
	}

	// Add a model to the scene and return the node it is in.
	// transform: if parent the transform is local to it.
	Node* AddModel(const void* in_model, const glm::mat4& in_transform, Node* in_parent = nullptr);

	// Set Parent to Node
	void SetParent(Node* in_node, Node* in_parent);

	// Delete the node in the scene
	// returns false if not found
	bool DeleteNode(Node* in_node);

	// Update input node's children
	virtual void Update(Node* in_node);

private:

	void computeWorldMatrix(Node* in_node, glm::mat4& in_parentWorldMatrix);
};