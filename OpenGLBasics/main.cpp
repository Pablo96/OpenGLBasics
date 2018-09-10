#include "main.h"
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
    Shader shader("res\\vertex.glsl", "res\\fragmentLit.glsl");
    Shader shaderLight("res\\vertex.glsl", "res\\fragmentLight.glsl");
    Texture tireTex("res\\Tire_df.png");
    Texture rimTex("res\\Rim_df.png");
    Texture tireTexS("res\\Tire_sp.png");
    Texture whiteTex("res\\white.bmp");
    Material tireMat = { &tireTex, &tireTexS, 5 };
    Material rimMat = { &rimTex, &whiteTex, 256 };
    std::vector<Material> materials = { tireMat, rimMat };
    Model model("res\\wheel.obj", &materials);
    Model cubeLight("res\\cube.obj");

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // LIGHTS POSITIONS
    glm::vec3 sunDir(0.0f, -1.0f, -1.0f);
    glm::vec3 pointLightPos[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f,  -3.0f)
    };

    // 2 draw calls per wheels so total draw calls = wheelsCount * 2 + 4 (cubeLights)
    const uint32 wheelsCount = 300;
    
    
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float) WIDTH / HEIGHT, 0.1f, 100.0f);
    glm::mat4 scaleMat = glm::scale(glm::vec3(.3f, .3f, .3f));
    glm::mat4 pointScaMat = glm::scale(glm::vec3(.1f, .1f, .1f));
    
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
            * glm::rotate(glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f)) * scaleMat;

        transforms.push_back(modelMat);
    }




    // Remember to bind the shader befor set the uniforms
    shader.bind();
    // Set Materials
    // bind the uniforms to the active unit texture (0 and 1 respectively)
    shader.setInt("material.diffuseTex", 0);
    shader.setInt("material.specular", 1);
 
    // Set sun light
    shader.setVec4f("sun.ambient", .2f, .2f, .2f);
    shader.setVec4f("sun.diffuse");
    shader.setVec4f("sun.specular");
    shader.setVec4f("sun.direction", sunDir.x, sunDir.y, sunDir.z, 0);

    /*
    // Set point lights
    for (uint32 i = 0; i < 4; i++)
    {
        shader.setVec4f(("pointLights[" + std::to_string(i) + "].position").c_str(), pointLightPos[i].x, pointLightPos[i].y, pointLightPos[i].z);
        shader.setVec4f(("pointLights[" + std::to_string(i) + "].diffuse").c_str(), .2f * i, .2f, 1.0f / (4-i));
        shader.setVec4f(("pointLights[" + std::to_string(i) + "].specular").c_str());
        shader.setVec4f(("pointLights[" + std::to_string(i) + "].ambient").c_str(), .0f, .0f, .0f);
        shader.setFloat(("pointLights[" + std::to_string(i) + "].constant").c_str(), 1);
        shader.setFloat(("pointLights[" + std::to_string(i) + "].linear").c_str(), .09);
        shader.setFloat(("pointLights[" + std::to_string(i) + "].quadratic").c_str(), .032);
    }
    */

    // Set lights objects color
    shaderLight.bind();
    shaderLight.setVec4f("ourColor");


    // Set clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        // TIME
        float currentFrame = (float) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::cout << deltaTime * 1000 << "ms\n";
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

        shaderLight.bind();
        for (uint32 i = 0; i < 4; i++)
        {
            glm::mat4 transform1 = PVmat * (glm::translate(pointLightPos[i]) * pointScaMat);
            shaderLight.setMat4f("transform", transform1);
            cubeLight.draw(shaderLight);
        }
        
        
        // Render the frame
        glfwSwapBuffers(window);
        // get the events
        glfwPollEvents();
        processInput(window);
        // flush cout msgs
        std::cout.flush();
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
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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