#include "main.h"
#include "Model.hpp"
#include <GLFW/glfw3.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define WIDTH 1280
#define HEIGHT 720
#define SHADOW_RES 1024
#define SHADOW_SIZE 1.0f

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

    glm::vec3 sunDir = glm::vec3(1, 1, 1);
	glm::vec3 sunPos = sunDir * 4.0f;

    // SHADERS
    Shader shader("res\\Shaders\\lit_normal.vert", "res\\Shaders\\lit_normal.frag");
    shader.bind();
    shader.setInt("diffuseMap", 0);
	shader.setInt("normals_map", 1);
	shader.setInt("shadow_map", 2);

	Shader skyShader("res\\Shaders\\skybox.vert","res\\Shaders\\skybox.frag");
	skyShader.bind();
	skyShader.setInt("diffuse", 0);

	Shader vfxShader("res\\Shaders\\vfx.vert","res\\Shaders\\vfx.frag");
	vfxShader.bind();
	vfxShader.setInt("screen", 0);

	Shader shadowMapShader("res\\Shaders\\shadow_mapping.vert", "res\\Shaders\\shadow_mapping.frag");
	// TEXTURES
	Texture2D* shadowTexture = new Texture2D("", false, SHADOW_RES, SHADOW_RES, true);
	Texture2D* screenTexture = new Texture2D("", false, WIDTH, HEIGHT);
	Texture2D* tireTexD =  new Texture2D("res\\Textures\\Tire_df.png");
	Texture2D* tireTexN =  new Texture2D("res\\Models\\Wheel\\Tire_LP_nm.png");
	Texture2D* rimTexD =  new Texture2D("res\\Textures\\Rim_df.png");
	Texture2D* rimTexN =  new Texture2D("res\\Models\\Wheel\\Rim_LP_nm.png");
	Texture2D* floorTexD =  new Texture2D("res\\Textures\\RedBrick\\brick_df.png");
	Texture2D* floorTexN =  new Texture2D("res\\Textures\\RedBrick\\brick_nm.png");
	Cubemap* skyTex = new Cubemap({
			"res/textures/cubemaps/hw_alps/alps_rt.tga",
			"res/textures/cubemaps/hw_alps/alps_lf.tga",
			"res/textures/cubemaps/hw_alps/alps_up.tga",
			"res/textures/cubemaps/hw_alps/alps_dn.tga",
			"res/textures/cubemaps/hw_alps/alps_ft.tga",
			"res/textures/cubemaps/hw_alps/alps_bk.tga",
		});

	// MATERIALS
	Material tireMat = { tireTexD, tireTexN, 0.5f, 27.0f};
    std::vector<Material> tireMats= { tireMat };
	Material rimMat = { rimTexD, rimTexN, 1.5f, 256.0f};
	std::vector<Material> rimMats = { rimMat };
	Material floorMaterial = { floorTexD, floorTexN, 0.0f, 12.0f};
	std::vector<Material> floorMaterials = { floorMaterial };
	Material skyMaterial = { skyTex, nullptr, 0, 0 };
	std::vector<Material> skyMaterials = { skyMaterial };
	
	// MODELS
    Model tireModel("res\\Models\\Wheel\\Tire_LP.obj", &tireMats);
	Model rimModel("res\\Models\\Wheel\\Rim_LP.obj", &rimMats);
	Model floor("res\\Models\\plane.obj", &floorMaterials);
	Model sky("res\\Models\\cube.obj", &skyMaterials);
	Model screenPlane("res\\Models\\planeZ.obj", nullptr);

	// TRANSFORMS
	glm::mat4 orthographic = glm::ortho(-SHADOW_SIZE, SHADOW_SIZE, -SHADOW_SIZE, SHADOW_SIZE, 5.0f, 19.0f);
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
	
	glm::mat4 floorMat = glm::translate(glm::vec3(0, -0.8, 0)) * glm::scale(glm::vec3(10.0f, 10.0f, 10.0f));
	glm::mat3 floorNMat = glm::transpose(glm::inverse(glm::mat3(floorMat)));
    
	glm::mat4 modelMat = glm::rotate(glm::radians(0.0f), glm::vec3(0, 1, 0));
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
	
	glm::mat4 sunView = glm::lookAt(sunPos, glm::vec3(0, 0, 0),	glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpace_mat = orthographic * sunView;
	glm::mat4 transform;
	float angle = 0.0f;

	
	// set shader's uniforms
	shadowMapShader.bind();
	shadowMapShader.setMat4f("lightspace_mat", lightSpace_mat);

	shader.bind();
	shader.setMat4f("lightspace_mat", lightSpace_mat);
	shader.setVec3f("lightPos", sunPos.x, sunPos.y, sunPos.z);
    
	// FRAMEBUFFERS
	Framebuffer shadowMap(shadowTexture, nullptr, true);
	Framebuffer screenVFX(screenTexture, nullptr);

	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);


	std::cout.flush();
	
	glClearColor(0.2f, 0.48f, 1.0f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        // FPS
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
		// LOGIC
		angle += 0.5f;
		angle = (angle > 360.0f) ? 0.0f : angle;
        camPos = cam.Position;
		modelMat = glm::rotate(glm::radians(angle), glm::vec3(1, 0, 0));
		
		// SHADOW MAP
		shadowMap.Bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOW_RES, SHADOW_RES);
		glCullFace(GL_FRONT);


		shadowMapShader.bind();
		shadowMapShader.setMat4f("model", modelMat);

		tireModel.draw();
		rimModel.draw();

        // RENDER SCENE
		//Framebuffer::Default();
		screenVFX.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, WIDTH, HEIGHT);
		glCullFace(GL_BACK);
		
		
		auto pv_mat = perspective * cam.GetViewMatrix();
		
		
		shader.bind();
		shader.setVec3f("viewPos", camPos.x, camPos.y, camPos.z);
		shader.setMat4f("pv_mat", pv_mat);
		
		shadowTexture->bind(2);

		shader.setMat4f("model", modelMat);
		tireModel.draw();
		rimModel.draw();

		shader.setMat4f("model", floorMat);
		floor.draw();
		
		skyShader.bind();
		skyShader.setMat4f("pv_transform", pv_mat);
		sky.draw();

		// VFX
		Framebuffer::Default();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		vfxShader.bind();
		screenTexture->bind();
		screenPlane.draw();
        
		
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