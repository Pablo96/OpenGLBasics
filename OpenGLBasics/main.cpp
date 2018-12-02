#include "main.h"
#include "Model.hpp"
#include <GLFW/glfw3.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define WIDTH 800
#define HEIGHT 450

glm::vec3 camPos (2.0f, 0.0f, 2.0f);
Camera cam(camPos, { 0.0f, 1.0f, 0.0f }, 231.499954f, 0.0f);
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

    glm::vec3 sunDir(1, 1, 1);
	glm::vec3 sunPos = sunDir * 1.8f;

    // SHADERS
    Shader shader("res\\Shaders\\4.normal_mapping.vs", "res\\Shaders\\4.normal_mapping.fs");
    shader.bind();
    shader.setInt("diffuseMap", 0);
	shader.setInt("normalMap", 1);
	

	// MODELS
	Texture tireTexD("res\\Textures\\Tire_df_lt.png");
	Texture tireTexN("res\\Models\\Wheel\\Tire_LP_nm.tga");
    Material tireMat = { &tireTexD, nullptr, &tireTexN, 0.5f, 27.0f};

	Texture rimTexD("res\\Textures\\Rim_df.png");
	Texture rimTexN("res\\Models\\Wheel\\Rim_LP_nm.tga");
	Material rimMat = { &rimTexD, nullptr, &rimTexN, 1.5f, 256.0f};

    std::vector<Material> tireMats= { tireMat };
    Model tireModel("res\\Models\\Wheel\\Tire_LP.obj", &tireMats);
	std::vector<Material> rimMats = { rimMat };
	Model rimModel("res\\Models\\Wheel\\Rim_LP.obj", &rimMats);

	Texture floorTexD("res\\Textures\\RedBrick\\brick_df.png");
	Texture floorTexN("res\\Textures\\RedBrick\\brick_nm.png");
	Material floorMaterial = { &floorTexD, nullptr, &floorTexN, 1.0f, 12.0f};
	
	std::vector<Material> floorMaterials = { floorMaterial };
	Model floor("res\\Models\\plane.obj", &floorMaterials);
    
	Texture sunD("res\\Textures\\white.bmp");
	Material sunMaterial = { &sunD, nullptr, nullptr, 1.0f, 1.0f };

	std::vector<Material> sunMaterials = { sunMaterial };
	Model sunModel("res\\Models\\sphere_lp.obj", &sunMaterials);

	// when instanced is 2 drawcalls 1 per mesh (wheel)
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    glm::mat4 PVmat = perspective * cam.GetViewMatrix();
	
	glm::mat4 floorMat = glm::translate(glm::vec3(0, -0.8, 0))
		* glm::scale(glm::vec3(10.0f, 10.0f, 10.0f));
	glm::mat3 floorNMat = glm::transpose(glm::inverse(glm::mat3(floorMat)));
    
	glm::mat4 modelMat = glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0));
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
	
	glm::mat4 sunMat = glm::translate(sunPos * 5.0f) * glm::scale(glm::vec3(0.2f));
	
	glm::mat4 transform;
	float angle = 0.0f;

	glClearColor(0.2f, 0.48f, 1.0f, 1.0f);
	
	// set shader's uniforms
	shader.bind();
	shader.setVec3f("lightPos", sunPos.x, sunPos.y, sunPos.z);
	shader.setMat4f("projection", perspective);
    
	
	
	
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
        camPos = cam.Position;
		modelMat = glm::rotate(glm::radians(angle), glm::vec3(1, 0, 0));
		

        // RENDER CALLS OR CODE
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.setVec3f("viewPos", camPos.x, camPos.y, camPos.z);
		shader.setMat4f("view", cam.GetViewMatrix());

		shader.setMat4f("model", sunMat);
		sunModel.draw(shader);

		shader.setMat4f("model", modelMat);
		tireModel.draw(shader);
		rimModel.draw(shader);

		shader.setMat4f("model", floorMat);
		floor.draw(shader);
		
        // Render the frame
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
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

    //cam.ProcessMouseMovement(xoffset, yoffset);
}