#include "main.h"
#include "Model.hpp"
#include <GLFW/glfw3.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define WIDTH 800
#define HEIGHT 640

glm::vec3 camPos(20.0f, -34.0f, -20.0f);
Camera cam(camPos, { 0.0f, 1.0f, 0.0f }, 125, 46);
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
int noInstance(GLFWwindow* window);
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
    //return noInstance(window);
}

int instanced(GLFWwindow* window)
{
    // SHADER
    Shader shader("res\\vertexInstanced.glsl", "res\\fragment.glsl");
    shader.bind();
    shader.setInt("diffuse", 0);

    // MODEL
    Texture tireTex("res\\Tire_df.png");
    Texture rimTex("res\\Rim_df.png");
    Material tireMat = { &tireTex, nullptr, 5 };
    Material rimMat = { &rimTex, nullptr, 256 };
    std::vector<Material> materials = { tireMat, rimMat };
    ModelInstanced model("res\\wheel.obj", &materials);

    
    // 2 draw calls per wheels so total draw calls = wheelsCount * 2 + 4 (cubeLights)
    const uint32 wheelsCount = 256;

    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    
    std::vector<glm::mat4> transforms;
    for (unsigned int i = 0; i < wheelsCount; i++)
    {
        float angle = 20.0f * i;
        glm::vec3 locVect = glm::vec3(
            rand() % 20 - 10.0f,
            rand() % 20 - 10.0f,
            rand() % 20
        );
        glm::mat4 modelMat = glm::translate(locVect)
            * glm::rotate(glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

        transforms.push_back(modelMat);
    }


    // Set clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    std::cout.flush();
    while (!glfwWindowShouldClose(window))
    {
        // TIME
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::cout << deltaTime * 1000 << "ms" << std::endl;
        // Logic
        glm::mat4 PVmat = perspective * cam.GetViewMatrix();
        camPos = cam.Position;

        // RENDER CALLS OR CODE
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();
        shader.setVec4f("viewPos", camPos.x, camPos.y, camPos.z);

        for (unsigned int i = 0; i < wheelsCount; i++)
        {
            glm::mat4 modelMat = transforms[i];
            glm::mat4 transform = PVmat * modelMat;
            std::string index = std::to_string(i);
            shader.setMat4f(("transforms[" + index + "]").c_str(), transform);
        }
        model.draw(shader, wheelsCount);

        // Render the frame
        glfwSwapBuffers(window);
        // get the events
        glfwPollEvents();
        processInput(window);
    }

    return 0;
}

int noInstance(GLFWwindow* window)
{
    Shader shader("res\\vertex.glsl", "res\\fragment.glsl");
    Texture tireTex("res\\Tire_df.png");
    Texture rimTex("res\\Rim_df.png");
    Texture tireTexS("res\\Tire_sp.png");
    Material tireMat = { &tireTex, nullptr, 5 };
    Material rimMat = { &rimTex, nullptr, 256 };
    std::vector<Material> materials = { tireMat, rimMat };
    Model model("res\\wheel.obj", &materials);

    
    // 2 draw calls per wheels so total draw calls = wheelsCount * 2 + 4 (cubeLights)
    const uint32 wheelsCount = 256;


    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

    std::vector<glm::mat4> transforms;
    for (unsigned int i = 0; i < wheelsCount; i++)
    {
        float angle = 20.0f * i;
        glm::vec3 locVect = glm::vec3(
            rand() % 20 - 10.0f,
            rand() % 20 - 10.0f,
            rand() % 20
        );
        glm::mat4 modelMat = glm::translate(locVect)
            * glm::rotate(glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

        transforms.push_back(modelMat);
    }




    // Remember to bind the shader befor set the uniforms
    shader.bind();
    // Set Materials
    // bind the uniforms to the active unit texture (0 and 1 respectively)
    shader.setInt("material.diffuseTex", 0);

    
    // Set clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        // TIME
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::cout << deltaTime * 1000 << "ms" << std::endl;
        // Logic
        glm::mat4 PVmat = perspective * cam.GetViewMatrix();

        // RENDER CALLS OR CODE
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        shader.bind();
        camPos = cam.Position;
        shader.setVec4f("viewPos", camPos.x, camPos.y, camPos.z);

        for (unsigned int i = 0; i < wheelsCount; i++)
        {
            glm::mat4 modelMat = transforms[i];
            glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(modelMat)));
            glm::mat4 transform = PVmat * modelMat;
            shader.setMat3f("normalMat", normalMat);
            shader.setMat3f("model", modelMat);
            shader.setMat4f("transform", transform);
            model.draw(shader);
        }

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