#pragma once
#include <unordered_set>
#include <GLM/glm.hpp>

class node
{
	friend class ktree;

private:
	glm::mat4 local_transform;
	glm::mat4 world_transform;

	void* model;

	node* parent;
	std::unordered_set<node*> children;
protected:
	node(glm::mat4& in_transform, void* in_model = nullptr, node* in_parent = nullptr)
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
	node* root;

public:
	ktree()
	{
		auto global_center = glm::mat4(1.0f);
		root = new node(global_center);
	}

	// Add a model to the scene and return the node it is in.
	// transform: if parent the transform is local to it.
	node* AddModel(void* in_model, glm::mat4& in_transform, node* in_parent = nullptr);

	// Set Parent to Node
	void SetParent(node* in_node, node* in_parent);

	// Delete the node in the scene
	// returns false if not found
	bool DeleteNode(node* in_node);

private:

	glm::mat4 computeWorldMatrix(node* in_node, glm::mat4& in_parentWorldMatrix);
};