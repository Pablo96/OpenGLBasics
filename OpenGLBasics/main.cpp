#include "main.h"
#include "Model.hpp"
#include <GLFW/glfw3.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define WIDTH 1280
#define HEIGHT 720

glm::vec3 camPos(-2.4f, 1.0f, -2.6f);
Camera cam(camPos, { 0.0f, 1.0f, 0.0f }, 49, -14);
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
int instanced(GLFWwindow* window);
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
    return instanced(window);
}

int instanced(GLFWwindow* window)
{
    glm::vec3 sunDir(1, 1, 1);
	glm::vec3 sunPos = sunDir * 1.8f;

    // SHADERS
    Shader shader("res\\Shaders\\vertexInstanced.vert", "res\\Shaders\\fragment_lit.frag");
    shader.bind();
    shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("material.normal", 2);

	shader.setVec4f("sun.direction", sunDir.x, sunDir.y, sunDir.z, 0);
    shader.setVec4f("sun.ambient", 0.1f, 0.1f, 0.12f);
    shader.setVec4f("sun.diffuse", 1.0f, 0.9f, 0.8f);
	shader.setFloat("sun.energy", 1.2f);
	

	Shader unlitShader("res\\Shaders\\vertexInstanced.vert", "res\\Shaders\\fragment_unlit.frag");
	unlitShader.setInt("diffuse", 0);
    
	
	// MODELS
	Texture tireTexD("res\\Textures\\Tire_df_lt.png");
    Texture tireTexS("res\\Textures\\Tire_sp.png");
	Texture tireTexN("res\\Textures\\Tire_nm.png");
    Material tireMat = { &tireTexD, &tireTexS, &tireTexN, 0.5f, 27.0f};

	Texture rimTexD("res\\Textures\\Rim_df.png");
	Texture rimTexS("res\\Textures\\Rim_sp.png");
	Texture rimTexN("res\\Textures\\Rim_nm.png");
	Material rimMat = { &rimTexD, &rimTexS, &rimTexN, 1.5f, 256.0f};

    std::vector<Material> materials = { tireMat, rimMat };
    ModelInstanced model("res\\Models\\wheel.obj", &materials, "Wheel");

	Texture floorTexD("res\\Textures\\RedBrick\\brick_df.png");
	Texture floorTexS("res\\Textures\\blue.bmp");
	Texture floorTexN("res\\Textures\\RedBrick\\brick_nm.png");
	Material floorMaterial = { &floorTexD, &floorTexS, &floorTexN, 1.0f, 12.0f};
	
	std::vector<Material> floorMaterials = { floorMaterial };
	ModelInstanced floor("res\\Models\\plane.obj", &floorMaterials);
    
	Texture sunD("res\\Textures\\white.bmp");
	Material sunMaterial = { &sunD, nullptr, nullptr, 1.0f, 1.0f };

	std::vector<Material> sunMaterials = { sunMaterial };
	ModelInstanced sunModel("res\\Models\\sphere_lp.obj", &sunMaterials);

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
		

        // RENDER CALLS OR CODE
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		unlitShader.bind();
		transform = PVmat * sunMat;
		sunModel.setTransforms(1, &transform, 0);
		sunModel.draw(unlitShader, 1);
			

		shader.bind();
		shader.setVec4f("viewPos", camPos.x, camPos.y, camPos.z);

		transform = PVmat * modelMat;
		model.setTransforms(1, &transform, 0);
		model.setTransforms(1, &modelMat, 1);
		model.setTransforms(1, &normalMat);
		model.draw(shader, 1);

		transform = PVmat * floorMat;
		floor.setTransforms(1, &transform, 0);
		floor.setTransforms(1, &floorMat, 1);
		floor.setTransforms(1, &floorNMat);
		floor.draw(shader, 1);
		
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