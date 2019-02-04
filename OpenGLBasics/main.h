#pragma once
#define GLEW_STATIC
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GLM/glm.hpp>
#include <GLM/gtx/transform.hpp>


typedef unsigned int uint32;

std::string getShaderSrc(const char* fileName)
{
    std::ifstream file;
    file.open(fileName);

    std::string output;
    std::string line;

    if (file.is_open())
    {
        while (file.good())
        {
            getline(file, line);
            output.append(line + "\n");
        }
    }
    else
    {
        std::cerr << "Unable to load shader: " << fileName << std::endl;
    }

    return output;
}

void shaderCompileStatus(uint32 shader)
{
    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, 0, infoLog);
        std::cout << "ERROR: Shader compilation failed.\n" << infoLog << "\n";
    }
}

class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // Get the shader sources and compile them
        auto stringSource = getShaderSrc(vertexPath);
        auto vertexShaderSource = stringSource.c_str();
        uint32 vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, 0);
        glCompileShader(vertexShader);
        shaderCompileStatus(vertexShader);

        stringSource = getShaderSrc(fragmentPath);
        const char* fragmentShaderSource = stringSource.c_str();
        uint32 fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, 0);
        glCompileShader(fragmentShader);
        shaderCompileStatus(fragmentShader);
        
        // create the shader program
        ID = glCreateProgram();
        // attach the shaders to the program
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        // link the atteched shaders to the program
        glLinkProgram(ID);

        // check for linking errors
        int  success;
        char infoLog[512];
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, 512, 0, infoLog);
            std::cout << "ERROR: Shader program linking failed.\n" << infoLog << "\n";
        }
        
        // if we dont detach them they wont be deleted until 
        // no program shader is using them
        glDetachShader(ID, vertexShader);
        glDetachShader(ID, fragmentShader);
        // if we dont use them in other shader program
        // we dont need the shaders once we've linked them
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    
    // use/activate the shader
    void bind()
    {
        glUseProgram(ID);
    }

    // utility uniform functions
    void setFloat(const char* name, const float value) const
    {
        glUniform1f(getLocation(name), value);
    }
    void setInt(const char* name, const int value) const
    {
        glUniform1i(getLocation(name), value);
    }
    void setVec4f(const char* name, const float x = 1.0f, const float y = 1.0f, const float z = 1.0f, const float w = 1.0f) const
    {
		auto loc = getLocation(name);
        glUniform4f(loc, x, y, z, w);
    }
    void setMat3f(const char* name, const glm::mat3& matrix) const
    {
        glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }
    void setMat4f(const char* name, const glm::mat4& matrix) const
    {
        glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, &matrix[0][0]);
    }
	void setMat4fVec(const char* name, const std::vector<glm::mat4>& matrixvec) const
	{
		auto loc = getLocation(name);
		glUniformMatrix4fv(loc, (GLsizei) matrixvec.size(), GL_FALSE, &matrixvec[0][0][0]);
	}
private:
    int getLocation(const char* name) const
    {
        return glGetUniformLocation(ID, name);
    }
};


class Texture
{
public:
    uint32 ID;

    Texture(const char* fileName)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(fileName, &width, &height, &nrChannels, 0);
        if (!data)
        {
            std::cout << "Failed to load texture" << std::endl;
            return;
        }

        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // generate texture
        GLenum channels = (nrChannels == 4) ? GL_SRGB_ALPHA : GL_SRGB;
		GLenum channelsImage = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, channelsImage, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    void bind(const uint32 unit = 0) const
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};


struct Material
{
	glm::vec4 diffuseCol;
    Texture* diffuse;
    Texture* specular;
	Texture* normal;
    float shininess;
};



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= 1.0f && Zoom <= 45.0f)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
        if (Zoom >= 45.0f)
            Zoom = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};