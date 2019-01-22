#include "main.h"
#include "Model.hpp"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
void setUpGUI(GLFWwindow* window);
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
	setUpGUI(window);

    glm::vec3 sunDir(1, 1, 1);
	glm::vec3 sunPos = sunDir * 1.8f;

    // SHADERS
    Shader shader("res\\Shaders\\vertexInstancedAnimated.vert", "res\\Shaders\\fragment_lit.frag");
    shader.bind();

	shader.setVec4f("sun.direction", sunDir.x, sunDir.y, sunDir.z, 0);
	shader.setVec4f("sun.position", sunPos.x, sunPos.y, sunPos.z, 1.0f),
    shader.setVec4f("sun.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec4f("sun.diffuse", 1.0f, 0.9f, 0.8f);
    shader.setVec4f("sun.specular");
	shader.setFloat("sun.energy", 10.5f);
	
	
	// Materials
    Material tireMat = { glm::vec4(0, 0, 1, 1), nullptr, nullptr, nullptr, 27.0f};
	Material rimMat = { glm::vec4(0, 1, 0, 1), nullptr, nullptr, nullptr, 256.0f};
	Material floorMaterial = { glm::vec4(0.5f, 0.5f, 0.5f, 1), nullptr, nullptr, nullptr, 5.0f };
	Material sunMaterial = { glm::vec4(1, 0.5f, 0, 1), nullptr, nullptr, nullptr, 1.0f };
	Material animatedMeshMaterial = { glm::vec4(0.8f, 0.8f, 0.8f, 1), nullptr, nullptr, nullptr, 1.0f };

	// Model Materials
    std::vector<Material> materials = { tireMat, rimMat };
	std::vector<Material> floorMaterials = { floorMaterial };
	std::vector<Material> sunMaterials = { sunMaterial };
	std::vector<Material> animatedMeshMaterials = { animatedMeshMaterial };

	
	// MODELS
    //Model model("res\\Models\\wheel.obj", &materials, "Wheel");
	//Model floor("res\\Models\\plane.obj", &floorMaterials);
	//Model sunModel("res\\Models\\sphere_lp.obj", &sunMaterials);
	Model animatedMesh("res\\Models\\AnimatedCubeArm\\AnimatedCubeArm.gltf", &animatedMeshMaterials);


	//#################################################
	//					MATRICES
	//#################################################

    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    glm::mat4 PVmat = perspective * cam.GetViewMatrix();
	
	glm::mat4 floorMat = glm::translate(glm::vec3(0, -0.8, 0))
					   * glm::scale(glm::vec3(10.0f, 10.0f, 10.0f));
    
	glm::mat4 modelMat = glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0));
	
	glm::mat4 sunMat = glm::translate(sunPos * 5.0f) * glm::scale(glm::vec3(0.2f));
	
	glm::mat4 animMeshMat = glm::translate(glm::vec3(-1, 0.0, 0.0));

	glm::mat4 transform;
	float angle = 0.0f;

	glm::vec3 translation(1, 0, 0);
	glClearColor(0.2f, 0.48f, 1.0f, 1.0f);
	
    std::cout.flush();
    while (!glfwWindowShouldClose(window))
    {
        // TIME
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Logic
		angle += .5f;
		angle = (angle > 360.0f) ? 0.0f : angle;
        PVmat = perspective * cam.GetViewMatrix();
        camPos = cam.Position;
		animMeshMat = glm::translate(translation);
		

        // RENDER CALLS OR CODE
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		/*
		unlitShader.bind();
		transform = PVmat * sunMat;
		sunModel.setTransforms(1, &transform, 0);
		sunModel.draw(unlitShader, 1, deltaTime);
			

		shader.bind();
		shader.setVec4f("viewPos", camPos.x, camPos.y, camPos.z);

		// set the shadow map
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		transform = PVmat * modelMat;
		model.setTransforms(1, &transform, 0);
		model.setTransforms(1, &modelMat, 1);
		model.draw(shader, 1, deltaTime);

		transform = PVmat * floorMat;
		floor.setTransforms(1, &transform, 0);
		floor.setTransforms(1, &floorMat, 1);
		floor.draw(shader, 1, deltaTime);
		*/
		transform = PVmat * animMeshMat;
		animatedMesh.setTransforms(1, &transform, 0);
		animatedMesh.setTransforms(1, &animMeshMat, 1);
		animatedMesh.draw(shader, 1, deltaTime);


		// GUI DATA
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			ImGui::SliderFloat3("Translation", &translation.x, 0, 1);            // Edit 1 float using a slider from 0.0f to 1.0f

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		// GUI render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Render the frame
        glfwSwapBuffers(window);
        // get the events
        glfwPollEvents();
        processInput(window);
    }

	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
    return 0;
}

void setUpGUI(GLFWwindow* window)
{
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
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

    cam.ProcessMouseMovement(xoffset, yoffset);
}