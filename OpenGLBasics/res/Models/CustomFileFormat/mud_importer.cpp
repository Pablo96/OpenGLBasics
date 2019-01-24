//###################################################################
//				BONE, POSE AND ANIMATION        WIP!!!
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

//###################################################################
//				VERTEX, MESH AND MODEL
//###################################################################

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uvCoord = glm::vec2(0, 0);
	glm::uvec4 indices = glm::vec4(0, 0, 0, 0); //Bones indices. Up to 4 bones
	glm::vec4 weights  = glm::vec4(0, 0, 0, 0);	//Bones weight. 1 per bone (4 total)
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
}

void loadModel(const std::string& path)
{
    tinyxml2::XMLDocument doc;
    //doc.LoadFile(path.c_str());
    doc.LoadFile("res/Models/CustomFileFormat/test.mudm");

    tinyxml2::XMLElement* meshNode = doc.FirstChildElement("model")->FirstChildElement("mesh");
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    

    for (auto vertexNode = meshNode->FirstChildElement("vertex"); vertexNode != nullptr; vertexNode = vertexNode->NextSiblingElement("vertex"))
    {
        Vertex vertex;
        
        // Position
        auto text = vertexNode->FirstChildElement("position")->FirstAttribute()->Value();
        {
            std::stringstream string(text);
            string >> vertex.pos.x;
            string >> vertex.pos.y;
            string >> vertex.pos.z;
        }

        // Normal
        text = vertexNode->FirstChildElement("normal")->FirstAttribute()->Value();
        {
            std::stringstream string(text);
            string >> vertex.normal.x;
            string >> vertex.normal.y;
            string >> vertex.normal.z;
        }

        // Bone Indices
        text = vertexNode->FirstChildElement("indices")->FirstAttribute()->Value();
        {
            std::stringstream string(text);
            string >> vertex.indices.x;
            string >> vertex.indices.y;
            string >> vertex.indices.z;
            string >> vertex.indices.w;
        }
        // Bone Weights
        text = vertexNode->FirstChildElement("weights")->FirstAttribute()->Value();
        {
            std::stringstream string(text);
            string >> vertex.weights.x;
            string >> vertex.weights.y;
            string >> vertex.weights.z;
            string >> vertex.weights.w;
        }
        vertices.emplace_back(vertex);
    }
    
    
    tinyxml2::XMLElement* indicesNode = meshNode->FirstChildElement("indices");
    uint32 count = std::strtoul(indicesNode->FirstAttribute()->Value(),  nullptr, 0);
    auto text = indicesNode->FindAttribute("values")->Value();
    std::stringstream string(text);
    uint32 index;
    for (size_t i = 0; i < count; i++)
    {
        string >> index;
        indices.emplace_back(index);
    }

    Mesh(vertices, indices);
}