#include "main.h"
#include "Model.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Shader_Legacy.hpp"
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <GLM/gtx/transform.hpp>
#include <math.h>
#include <ostream>

#define BUFFER_SIZE 1024
#define WIDTH 1280
#define HEIGHT 720

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	os << '(' << vec.x << ',' << vec.y << ',' <<  vec.z << ')';
	return os;
}

glm::vec3 camPos(2.47f, 1.01f, 2.06f);
Camera cam(camPos, { 0.0f, 1.0f, 0.0f }, -137.7f, -17.9f);
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool firstMouse = true;

void initLog()
{
    std::ios_base::sync_with_stdio(false);
    setvbuf(stdout, 0, _IOLBF, BUFFER_SIZE);
}

int createWindow(GLFWwindow** window);
int configOpenGL();
int run(GLFWwindow* window);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main(int argc, char** argv)
{
    initLog();

    GLFWwindow* window = nullptr;
    if (createWindow(&window) || configOpenGL())
    {
        return 1;
    }

    if(run(window))
    {
        return 1;
    }

    glfwTerminate();
    return 0;
}

int run(GLFWwindow* window)
{
    // SHADERS
	// Note: They should be pointer since if they are copied the destructor is not called
    Shader* v_Shader = new Shader("res\\Shaders\\vertex_basic.vert", SHADER_TYPE::VERTEX);
	Shader* f_Shader = new Shader("res\\Shaders\\fragment_lit.frag", SHADER_TYPE::FRAGMENT);

	std::vector<Shader*> shaders = { v_Shader, f_Shader };
	Program shader(shaders);
	shader.setInt("diffuse", 0);

	// MODELS
    Texture tireTexD("res\\Textures\\Tire_df.png");
    Texture tireTexS("res\\Textures\\Tire_sp.png");
	Texture tireTexN("res\\Textures\\Tire_nm_inv.png");
    Material tireMat = { &tireTexD, &tireTexS, &tireTexN, 27.0f};

	Texture rimTexD("res\\Textures\\Rim_df.png");
	Texture rimTexS("res\\Textures\\Rim_sp.png");
	Texture rimTexN("res\\Textures\\Rim_nm.png");
	Material rimMat = { &rimTexD, &rimTexS, &rimTexN, 256.0f};

    std::vector<Material> materials = { tireMat, rimMat };
    Mesh model("res\\Models\\wheel.mudm");
	model.setMaterials(materials);
	
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    glm::mat4 PVmat = perspective * cam.GetViewMatrix();
	
	
	glm::mat4 modelMat = glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0));
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
	
	glm::mat4 transform;
	float angle = 0.0f;
	

	
    std::cout.flush();
    while (!glfwWindowShouldClose(window))
    {
        // TIME
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::cout << deltaTime * 1000 << "ms" << std::endl;

        // Logic
		angle += .5f;
		angle = (angle > 360.0f) ? 0.0f : angle;
        PVmat = perspective * cam.GetViewMatrix();
        camPos = cam.Position;
		modelMat = glm::rotate(glm::radians(angle), glm::vec3(1, 0, 0));
		normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
		

        // RENDER CALLS OR CODE
		{
			glClearColor(0.2f, 0.48f, 1.0f, 1.0f);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shader.bind();
			shader.setVec4f("viewPos", camPos.x, camPos.y, camPos.z);
			shader.setMat4f("transform", transform);
			shader.setMat4f("model", modelMat) ;

			transform = PVmat * modelMat;
			model.draw(*reinterpret_cast<Program*>(&shader));
		}

        // Present the frame
        glfwSwapBuffers(window);
        // get the events
        glfwPollEvents();
        processInput(window);
    }

    return 0;
}

void processInput(GLFWwindow* window)
{
    /*
    Ask glfw is key was pressed and do stuff acorrding
    */
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        deltaTime *= 4;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.ProcessKeyboard(RIGHT, deltaTime);
}

int createWindow(GLFWwindow** window)
{
    /*
    Create the window
    */


    if (!glfwInit())
    {
        std::cerr << "ERROR: couldnt init glfw." << std::endl;
        return 1;
    }
    // per channel bit are default 8
    //glfwWindowHint(GLFW_RED_BITS, 8);

    // depth buffer bits are default 24
    //glfwWindowHint(GLFW_DEPTH_BITS, 16);
    
    // opengl 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // tell glfw we want to use core profile(no legacy stuff)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    *window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Test", NULL, NULL);
    if (!*window)
    {
        std::cerr << "ERROR: couldnt create window." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(*window);


    glfwSetCursorPosCallback(*window, mouse_callback);
    // if not set to disable camera doesnt work well
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // 0 = disable vsync
    glfwSwapInterval(0);
    return 0;
}

int configOpenGL()
{
    /*
    Configure opengl
    */
    if (glewInit())
    {
        std::cerr << "ERROR: couldnt init glew." << std::endl;
        return 1;
    }

    // opengl aspect ratio
    glViewport(0, 0, WIDTH, HEIGHT);

    // enable z-buffer so it doesn overlap faces
    glEnable(GL_DEPTH_TEST);
    // enable cull face making discard faces that doesnt face the camera
    glEnable(GL_CULL_FACE);
    // in this case fback faces are the ones not rendered
    glCullFace(GL_BACK);
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float) xpos;
        lastY = (float) ypos;
        firstMouse = false;
    }

    float xoffset = float(xpos - lastX);
    float yoffset = float(lastY - ypos); // reversed since y-coordinates go from bottom to top

    lastX = (float) xpos;
    lastY = (float) ypos;

    cam.ProcessMouseMovement(xoffset, yoffset);
}