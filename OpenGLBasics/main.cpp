#include "main.h"
#include "Model.hpp"
#include <GLFW/glfw3.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define WIDTH 1280
#define HEIGHT 720
#define SHADOW_RES 1024
#define SHADOW_SIZE 1.0f

glm::vec3 camPos (8.0f, 3.0f, 8.0f);
Camera cam(camPos, { 0.0f, 1.0f, 0.0f }, 231.499954f, -22.0f);
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
    glm::vec3 backgroundColor(0.2f, 0.5f, 1.0f);

    // SHADERS
	Shader emitShader("res/Shaders/unlit.vert", "res/Shaders/unlit.frag");
	emitShader.bind();
    emitShader.setVec3f("color", 0.01f, 0, 0.5f);

	// TRANSFORMS
    float scale = 1.0f;
    glm::vec3 position(0.0f, 0.0f, 0.0f);
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
	glm::mat4 modelMat = glm::translate(position) * glm::scale(glm::vec3(1.0f, 1.0f, 1.0f) * scale);

    // MODEL
    float vertices[] =
    {
        -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, 1.0f,
         1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 1.0f,

        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
    };

    uint32 indices[] =
    {
        // -Z face
        0, 1, 2,
        0, 2, 3,

        // +Z face
        4, 6, 5,
        4, 7, 6,

        // -X face
        4, 0, 3,
        4, 3, 7,
        
        //// +X face
        5, 2, 1,
        5, 6, 2,
        
        //// -Y face
        0, 4, 5,
        0, 5, 1,
        
        //// +Y face
        2, 6, 7,
        2, 7, 3,
    };


    size_t index_count = sizeof(indices) / sizeof(uint32);

    uint32 model;
    {
        uint32 buffers[2];
        // Generate the buffers
        glGenVertexArrays(1, &model);
        glGenBuffers(2, buffers);

        // Bind the Array Object
        glBindVertexArray(model);


        // Bind Vertex Buffer to the Array Object
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        // Reserve memory and Send data to the Vertex Buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

	std::cout.flush();
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        // FPS
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // RENDER SCENE
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		
		auto pv_mat = perspective * cam.GetViewMatrix();
		
		
		emitShader.bind();
		emitShader.setMat4f("pv_mat", pv_mat);
		
		emitShader.setMat4f("model", modelMat);
        
        glBindVertexArray(model);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

		// PRESENT THE FRAME
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