#pragma once
#include "k_tree.hpp"
#include "Model.hpp"

enum SCENENODE_TYPE : unsigned char
{
	EMPTY = 0,
	MODEL,
	LIGHT,
	CAMERA
};


class SceneNode : public Node
{
	friend Scene;

private:

	SCENENODE_TYPE type;
	glm::mat4 view_matrix;
	bool is_camera;

protected:

	SceneNode(glm::mat4& in_transform, SCENENODE_TYPE in_type, void* in_model = nullptr,
		SceneNode* in_parent = nullptr, bool set_view_node = false)
		: Node(in_transform, in_model, in_parent), type(in_type), is_camera(set_view_node)
	{
		UpdateViewMatrix();
	}

	void UpdateViewMatrix()
	{
		if (is_camera)
		{
			// calc view matrix
		}
	}

	void Update()
	{
		UpdateViewMatrix();
		Node::Update;
	}
};

class Scene
{
	ktree scene_graph;
	SceneNode* active_cam;

public:
	Scene() = default;

	Node* AddModel(const Model* in_model, const glm::mat4& in_transform = glm::mat4(1.0f))
	{
		return scene_graph.AddModel(in_model, in_transform);
	}
	
	bool DeleteModel(Node* in_node)
	{
		return scene_graph.DeleteNode(in_node);
	}
	
	void DrawScene()
	{

	}

	void UpdateNode(Node* in_node)
	{
		scene_graph.Update(in_node);
	}

	void UpdateNodes(std::vector<Node*>& in_nodes)
	{
		for (auto in_node : in_nodes)
		{
			UpdateNode(in_node);
		}
	}
};