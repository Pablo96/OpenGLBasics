#include "k_tree.hpp"

Node * ktree::AddModel(const void * in_model, const glm::mat4 & in_transform, Node * in_parent)
{
	Node* parent = (!in_parent) ? root : in_parent;

	auto node_model = new Node(in_transform, in_model, parent);
	
	computeWorldMatrix(node_model, parent->world_transform);

	return node_model;
}

void ktree::SetParent(Node * in_node, Node * in_parent)
{
	// if node has parent get node out of parent's children
	if (in_node->parent)
	{
		auto parent = in_node->parent;
		parent->children.erase(in_node);
	}
	
	// set double link
	in_node->parent = in_parent;
	in_parent->children.emplace(in_node);

	computeWorldMatrix(in_node, in_parent->world_transform);
}

bool ktree::DeleteNode(Node * in_node)
{
	// delete from children list
	auto erased = in_node->parent->children.erase(in_node);
	// free memory
	delete in_node;
	// anullate pointer
	in_node = nullptr;

	return (bool)erased;
}

void ktree::Update(Node * in_node)
{
	computeWorldMatrix(in_node, in_node->parent->world_transform);
}

void ktree::computeWorldMatrix(Node * in_node, glm::mat4 & in_parentWorldMatrix)
{
	glm::mat4 world_transform = in_parentWorldMatrix * in_node->local_transform;
	in_node->world_transform = world_transform;

	for (auto child : in_node->children)
	{
		computeWorldMatrix(child, world_transform);
	}
}
