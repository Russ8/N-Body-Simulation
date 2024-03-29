// OpenCLTest.cpp : Defines the entry point for the console application.
//
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define SUCCESS 0
#define _CRT_SECURE_NO_DEPRECATE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <Windows.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "Shader.h"
#include "camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;
float fov = 100.0f;
size_t global_work_size = 10000;

bool firstMouse = true;
int lastX, lastY;

cl_uint counter = 0;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool toggle = 0;
int timer = 0;

Camera * camera;

int nParticles = 5000;


void testStatus(int status, const char *errorMsg)
{
	if (status != SUCCESS)
	{
		if (errorMsg == NULL)
		{
			printf("Error\n");
		}
		else
		{
			printf("Error: %s", errorMsg);
		}
		exit(EXIT_FAILURE);
	}
}



unsigned int loadRandArrayToVao(int nParticles, int size) {
	float * testVao = new float[nParticles * 3];


	for (size_t i = 0; i < nParticles * 3; i++)
	{
		testVao[i] = rand() % size - (size/2);
	}



	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(testVao), testVao, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	return VAO;
}


int main()
{

	camera = new Camera(glm::vec3(15000.0f, 10.2f, 0.0f));
	/*
	bool failed = false;
	

	std::ifstream nfile("num_particles.txt");
	std::string nsource;

	char line[256];
	nfile.getline(line, 255, '\n');

	nsource += line;

	std::cout << nsource << "\n";
	*/
	nParticles = 40000;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "opencl", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//aspect_ratio = float(SCR_HEIGHT) / float(SCR_WIDTH);
	//scale = tan((fov / 2) * (CL_M_PI / 180.0));


	//******************PLATFORMS*********************
	//PLATFORM: A platform is a specific OpenCL implementation, for instance AMD APP, NVIDIA or Intel OpenCL
	//query the set of OpenCL platforms and choose one or more of them to use in the application
	cl_platform_id platform;
	//clGetPlatformIDs(1, &platform, NULL);
	cl_device_id device;

	// Get first OpenCL platform.
	cl_int err = clGetPlatformIDs(1, &platform, nullptr);
	if (err != CL_SUCCESS)
	{
		std::cout << "Couldn't find an OpenCL platform!" << std::endl;
		//getchar();
		return -1;
	}

	// Get first OpenCL gpu device.
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
	if (err != CL_SUCCESS)
	{
		std::cout << "Couldn't find a OpenCL enabled GPU!" << std::endl;
		//getchar();
		return -1;
	}


	//*********************DEVICES********************
	//devices are the actual processors (CPU, GPU etc.) that perform calculations.
	//We pass our platform as the 1st parameter and the type of the device as the 2nd.
	//cl_device_id device;
	//clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);


	//Additional attributes to OpenCL context creation
	//which associate an OpenGL context with the OpenCL context 

	//Specifies a list of context property names and their corresponding values. . .
	cl_context_properties props[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
		0
	};

	cl_int status;


	//*********************CONTEXTS********************
	//A context is a platform with a set of available devices for that platform.
	
	cl_context context = clCreateContext(props, 1, &device, NULL, NULL, &err);
	if (!context || err != CL_SUCCESS)
	{
		std::cout << "Couldn't create an OpenCL context." << std::endl;
		getchar();
		return -1;
	}
	

	float * testVao = new float[nParticles * 3];
	float * mass = new float[nParticles];

	//particle positions
	for (size_t i = 0; i < nParticles; i++)
	{

		testVao[i * 3] = ((rand()*rand()) % (2 * nParticles)) - (nParticles);
		testVao[i*3+1] = ((rand()*rand()) % (2 * nParticles)) - (nParticles);
		testVao[i*3+2] = ((rand()*rand()) % (2 * nParticles)) - (nParticles);
		//std::cout << testVao[i * 3] << " ";
		float dist = sqrt((testVao[i * 3] * testVao[i * 3]) + (testVao[i * 3+1] * testVao[i * 3+1]) + (testVao[i * 3+2] * testVao[i * 3+2]));
		
		while (dist > 0.7f*nParticles) {
			
			testVao[i * 3] = ((rand()*rand()) % (2 * nParticles)) - (nParticles);
			testVao[i * 3 + 1] = ((rand()*rand()) % (2 * nParticles)) - (nParticles);
			testVao[i * 3 + 2] = ((rand()*rand()) % (2 * nParticles)) - (nParticles);

			dist = sqrt( (testVao[i * 3] * testVao[i * 3]) + (testVao[i * 3 + 1] * testVao[i * 3 + 1]) + (testVao[i * 3 + 2] * testVao[i * 3 + 2]) );
			
		}

		testVao[i * 3] = testVao[i * 3] / 30.0f;

		testVao[i * 3+1] = testVao[i * 3+1] / 3.0f;
		testVao[i * 3+2] = testVao[i * 3+2] / 3.0f;
	}

	//particle mass
	for (size_t i = 0; i < nParticles; i++)
	{
		mass[i] = ((rand() % 9) / 1.0f ) + 1.0f;
	}


	//create opencl buffer from VBO
	GLuint outputVBO;
	cl_mem outputVBO_cl;

	//create VAO
	unsigned int bufferVAO; 
	glGenVertexArrays(1, &bufferVAO);
	glBindVertexArray(bufferVAO);

	// create buffer object
	glGenBuffers(1, &outputVBO);
	glBindBuffer(GL_ARRAY_BUFFER, outputVBO);

	// initialize buffer object
	unsigned int size = nParticles * 3 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, size, testVao, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// create OpenCL buffer from GL VBO
	outputVBO_cl = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE,  //write only?
		outputVBO, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);



	//create particle mass vbo
	GLuint massVBO;
	cl_mem massVBO_cl;

	// create buffer object
	glGenBuffers(1, &massVBO);
	glBindBuffer(GL_ARRAY_BUFFER, massVBO);

	// initialize buffer object
	size = nParticles * 1 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, size, mass, GL_STATIC_READ); //?
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	// create OpenCL buffer from GL VBO
	massVBO_cl = clCreateFromGLBuffer(context, CL_MEM_READ_ONLY,  //write only?
		massVBO, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//eg texture
	//cl_mem mem = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texturedQuad->getTextureId(), NULL);

	//******************COMMAND QUEUE***********************
	/*Command-queues are the core of OpenCL. A platform defines a context
	that contains one or more compute devices.For each compute device
	there is one or more command - queues.Commands submitted to these
	queues carry out the work of an OpenCL program. */
	cl_command_queue queue = clCreateCommandQueue(context, device, 0, NULL);

	//cl_command_queue queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, NULL);

	// ***************** Perform runtime source compilation, and obtain kernel entry point ****************
	
	std::ifstream file("kernel.cl");
	std::string source;
	while (!file.eof()) {
		char line[256];
		file.getline(line, 255, '\n');

		std::cout << line << "\n";


		source += line;
		source += '\n';
	}
	const char* str = source.c_str();

	//****************** CL PROGRAM **************************
	//A program is a collection of one or more kernels plus optionally supporting functions

	cl_program program = clCreateProgramWithSource(context, 1, &str, NULL, NULL);

	cl_int result = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (result) {
		std::cout << "Error during compilation! (" << result << ")" << std::endl;
	}

	size_t len;
	char *buffer;
	int clStatus = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
	buffer = (char *)malloc(len);
	clStatus = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
	fprintf(stdout, "Kernel build log: %s\n", buffer);
	free(buffer);
	
	//******************* openCL KERNEL *************************
	/*A Kernel is an executable entity (not necessarily compiled, since you can have built-in kernels
	that represent piece of hardware (e.g. Video Motion Estimation kernels on Intel hardware)), you
	can bind its arguments and submit them to various queues for execution.*/

	cl_kernel kernel_main = clCreateKernel(program, "main_", NULL);

	//Create data buffers.
	cl_mem velocities = clCreateBuffer(context, CL_MEM_READ_WRITE, 3 * nParticles * sizeof(float), NULL, 0);
	//cl_mem time = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_uint), NULL, 0);

	//clSetKernelArg is Used to set the argument value for a specific argument of a kernel.
	//status = clSetKernelArg(kernel_image, 0, sizeof(inputArray), &inputArray);
	//testStatus(status, "clSetKernelArg0 e");

	status = clSetKernelArg(kernel_main, 0, sizeof(outputVBO_cl), &outputVBO_cl);
	testStatus(status, "clSetKernelArg1 e");

	status = clSetKernelArg(kernel_main, 1, sizeof(velocities), &velocities);
	testStatus(status, "clSetKernelArg1 e");

	status = clSetKernelArg(kernel_main, 2, sizeof(massVBO_cl), &massVBO_cl);
	testStatus(status, "clSetKernelArg1 e");

	//Fill input data buffers OpenCL. clEnqueueMapBuffer is a mechanism for accessing memory objects 

	//enque  gets the location of the shared memory, so we can modify it from cpu
	
	cl_float *velocitiesPtr = (cl_float *)clEnqueueMapBuffer(queue,
		velocities,
		CL_TRUE,
		CL_MAP_WRITE,
		0,
		nParticles * 3 * sizeof(float),
		0, NULL, NULL, NULL);
	
	//set initial particle velocities

	for (size_t i = 0; i < nParticles; i++)
	{
		float damper = 200.0f;
		velocitiesPtr[i*3] = -testVao[i * 3 + 1] / damper;
		velocitiesPtr[i * 3+1] = testVao[i * 3 + 2] / damper;
		velocitiesPtr[i * 3+2] = testVao[i * 3] / damper;
	}

	/* To release any additional resources and to tell the OpenCL runtime that
	buffer mapping is no longer required, the following command can be
	used : */

	clEnqueueUnmapMemObject(queue, velocities, velocitiesPtr, 0, 0, 0);



	//load the openGL shaders
	Shader * particleShader = new Shader("particle.vs", "particle.fs", "particle.gs");


	//vsync:
	glfwSwapInterval(0);

	//float testVao[] = {
		//10, 10, 0, -10, 0, -10, 10, -20, 0
	//};
	//int nParticles = 3;
	/*
	const int nVAOS = 10;
	int nParticles = 1000000;
	int size = 10000;
	int vaos[nVAOS];
	for (size_t i = 0; i < nVAOS; i++)
	{
		vaos[i] = loadRandArrayToVao(nParticles, size);
	}
	*/

	//draw image
	while (!glfwWindowShouldClose(window))
	{
		counter++;
		//std::cout << "SDFSDFS" << std::endl;
		
		//openCL stuff:
		// Execute the kernel clEnqueueNDRangeKernel - Enqueues a command to execute a kernel on a device.

		//where the computation happens

		//Acquiring the ownership via clEnqueueAcquireGLObjects :
		glFinish();
		status = clEnqueueAcquireGLObjects(queue, 1, &outputVBO_cl, 0, 0, NULL);
		status = clEnqueueAcquireGLObjects(queue, 1, &massVBO_cl, 0, 0, NULL);

		testStatus(status, "clEnqueueAcquireGLObjects");

		size_t global_dim[1];
		global_dim[0] = nParticles;

		status = clEnqueueNDRangeKernel(queue, kernel_main, 1, NULL, global_dim, NULL, 0, NULL, NULL);
		testStatus(status, "clEnqueueNDRangeKernel");

		clFinish(queue);
		clEnqueueReleaseGLObjects(queue, 1, &outputVBO_cl, 0, 0, NULL);
		clEnqueueReleaseGLObjects(queue, 1, &massVBO_cl, 0, 0, NULL);

		//status = clEnqueueAcquireGLObjects(queue, 1, &outputVBO_cl, 0, 0, NULL);
		//Opengl stuff

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		//create projection, view matrices
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 40000.0f);

		//get view matrix
		glm::mat4 view;
		view = camera->GetViewMatrix();

		///setup shader
		particleShader->use();
		particleShader->setMat4("projection", projection);
		particleShader->setMat4("view", view);
		/*
		//draw vao
		for (size_t i = 0; i < nVAOS; i++)
		{
			glBindVertexArray(vaos[i]);

			glDrawArrays(GL_POINTS, 0, nParticles);
		}
		*/

		glBindVertexArray(bufferVAO);

		glDrawArrays(GL_POINTS, 0, nParticles);

		//input
		processInput(window);

		glfwSwapBuffers(window);
		glfwPollEvents();
		// glfw: terminate, clearing all previously allocated GLFW resources.
		// ------------------------------------------------------------------
	}
	glfwTerminate();

	return 0;

}

//setup input
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {

		camera->ProcessKeyboard(FORWARD, deltaTime);
	
		
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera->ProcessKeyboard(SPEED_ADJ, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		camera->ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera->ProcessKeyboard(W, deltaTime);
	}
			

		
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera->ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera->ProcessKeyboard(SLOW, deltaTime);
	

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	//SCR_WIDTH = width;
	//SCR_HEIGHT = height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{

		//std::cout << "mouse: " << xpos << std::endl;
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		camera->ProcessMouseMovement(xpos, ypos, xoffset, yoffset);
	}

//scroll wheel input
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}