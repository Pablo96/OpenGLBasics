#include "main.h"
#include "Model.hpp"
#include <GLFW/glfw3.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define WIDTH 800
#define HEIGHT 640

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
    //return noInstance(window);
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
	shader.setInt("shadowMap", 3);

	shader.setVec4f("sun.direction", sunDir.x, sunDir.y, sunDir.z, 0);
    shader.setVec4f("sun.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec4f("sun.diffuse", 1.0f, 0.9f, 0.8f);
    shader.setVec4f("sun.specular");
	shader.setFloat("sun.energy", 1.5f);
	

	Shader r2TexShader("res\\Shaders\\renderToTexture.vert", "res\\Shaders\\renderToTexture.frag");
	r2TexShader.bind();
	r2TexShader.setInt("screenTexture", 0);

	Shader shadowMap("res\\Shaders\\shadowMap.vert", "res\\Shaders\\empty.frag");

	Shader unlitShader("res\\Shaders\\vertexInstanced.vert", "res\\Shaders\\fragment_unlit.frag");
	unlitShader.setInt("diffuse", 0);
    
	
	// MODELS
    Texture tireTexD("res\\Textures\\Tire_df_lt.png");
    Texture tireTexS("res\\Textures\\Tire_sp.png");
	Texture tireTexN("res\\Textures\\Tire_nm_inv.png");
    Material tireMat = { &tireTexD, &tireTexS, &tireTexN, 27.0f};

	Texture rimTexD("res\\Textures\\Rim_df.png");
	Texture rimTexS("res\\Textures\\Rim_sp.png");
	Texture rimTexN("res\\Textures\\Rim_nm.png");
	Material rimMat = { &rimTexD, &rimTexS, &rimTexN, 256.0f};

    std::vector<Material> materials = { tireMat, rimMat };
    ModelInstanced model("res\\Models\\wheel.obj", &materials);

	Texture floorTexD("res\\Textures\\RedBrick\\brick_df.png");
	Texture floorTexS("res\\Textures\\blue.bmp");
	Texture floorTexN("res\\Textures\\RedBrick\\brick_nm.png");
	Material floorMaterial = { &floorTexD, &floorTexS, &floorTexN, 5.0f };
	
	std::vector<Material> floorMaterials = { floorMaterial };
	ModelInstanced floor("res\\Models\\plane.obj", &floorMaterials);
    
	Texture sunD("res\\Textures\\white.bmp");
	Material sunMaterial = { &sunD, nullptr, nullptr, 1.0f };

	std::vector<Material> sunMaterials = { sunMaterial };
	ModelInstanced sunModel("res\\Models\\sphere_lp.obj", &sunMaterials);

	// when instanced is 2 drawcalls 1 per mesh (wheel)
    const uint32 wheelsCount = 1;

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

	// Screen plane
	uint32 vao;
	{

		float vertices[] = 
		{
			// pos		// texcoord
			-1.0f, -1.0f,		0.0f, 0.0f,
			 1.0f, -1.0f,		1.0f, 0.0f,
			 1.0f,  1.0f, 		1.0f, 1.0f,
			-1.0f,  1.0f,		0.0f, 1.0f
		};

		uint32 indices[] = 
		{
			0, 1, 2,
			2, 3, 0
		};

		uint32 VBO, EBO;
		// Generate the buffers
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		
		// Bind the Array Object
		glBindVertexArray(vao);



		// Bind Vertex Buffer to the Array Object
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// Reserve memory and Send data to the Vertex Buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, vertices, GL_STATIC_DRAW);


		// Bind Element Buffer to the Array Object
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		// Reserve memory and Send data to the Elment Buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int),
			indices, GL_STATIC_DRAW);



		// Set the atribute pointers
		// vertex positions
		glEnableVertexAttribArray(0);
		// vertex texture coords
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	}

	
	// FRAMEBUFFER
	uint32 fbo;
	glGenFramebuffers(1, &fbo);
	
	//GL_FRAMEBUFFER: all the next read and write framebuffer operations will affect the currently bound framebuffer.
	//GL_READ_FRAMEBUFFER: Read only.
	//GL_DRAW_FRAMEBUFFER: Wtite only.
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Create a texxture attachment
	uint32 texture;
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// Only reserve memory but dont pass data sincce it will be filled by the framebuffer renders.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// attach it to the framebuffer
	// level: mipmap level.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	// Create a render buffer (the render buffer is a buffer like the texture but raw{not converted to texture format}
	// so it is faster to write and copy but cant be easyly accessed after.
	uint32 rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	// bind the render buffer to the framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);


	// Check if it is currently bound since it need to have at least one color attachment.
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Warning: Framebuffer is not complete!\n";
	}

	// only the default framebuffer (0) have visual output. All the others are for offscreen rendering.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDeleteFramebuffers(1, &fbo);
	

	//////////////////////////////////////
	//			SHADOW MAPPING			//
	//////////////////////////////////////

	// the depth map framebuffer
	uint32 depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// 2D texture for the framebuffer
	const int shadowWidth = 256;
	const int shadowHeight = shadowWidth;
	uint32 depthMap;
	{
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		// Only reserve memory but dont pass data sincce it will be filled by the framebuffer renders.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// fix outside texture coord shadows
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	// bind the depth framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	// attach the depthmap to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	// tell opengl the framebuffer will not have draw attachment else wont be complete
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	// unbind the framebuffer binding the default one
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// create the view and projection matrix(directional light is ortho proj)
	const float near_plane = 1.0f, far_plane = 6.0f;
	const float shadowSize = 0.95f;
	glm::mat4 lightProjMat = glm::ortho(-shadowSize, shadowSize, -shadowSize, shadowSize, near_plane, far_plane);
	glm::mat4 lightViewMat = glm::lookAt(sunPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 PVmatLight = lightProjMat * lightViewMat;
	shadowMap.bind();
	shadowMap.setMat4f("lightSpaceMatrix", PVmatLight);
	shader.bind();
	shader.setMat4f("lightSpaceMatrix", PVmatLight);

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

		// Render the shadow map
		{
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glViewport(0, 0, shadowWidth, shadowHeight);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_DEPTH_BUFFER_BIT);

			shadowMap.bind();

			shadowMap.setMat4f("model", modelMat);
			model.draw(shader, 1);

			shadowMap.setMat4f("model", floorMat);
			floor.draw(shader, 1);
		}
		

		// Render to texture
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glEnable(GL_DEPTH_TEST);
			glViewport(0, 0, WIDTH, HEIGHT);
			glClearColor(0.2f, 0.48f, 1.0f, 1.0f);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			unlitShader.bind();
			transform = PVmat * sunMat;
			sunModel.setTransforms(1, &transform, 0);
			sunModel.draw(unlitShader, 1);
			

			shader.bind();
			shader.setVec4f("viewPos", camPos.x, camPos.y, camPos.z);

			// set the shadow map
			glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D, depthMap);

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
		}
		

		// render to screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		r2TexShader.bind();
		glBindVertexArray(vao);

		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMap);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
		

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