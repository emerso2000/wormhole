#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "shaderClass.h"

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 800;

const unsigned short OPENGL_MAJOR_VERSION = 4;
const unsigned short OPENGL_MINOR_VERSION = 6;

bool vSync = true;

GLfloat vertices[] = {
	-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
	-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
};

GLuint indices[] = {
	0, 2, 1,
	0, 3, 2
};

struct CameraData {
	glm::vec3 cam_origin;
	float padding1;
	glm::vec3 forward;
	float padding2;
	glm::vec3 right;
	float padding3;
	glm::vec3 up;
	float fov;
} camera;

void initialSetup() {
	camera.cam_origin = glm::vec3(0, 0, -1);
	camera.forward = glm::vec3(0.0f, 0.0f, 1.0f);
	camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
	camera.right = glm::cross(camera.forward, camera.up);
	camera.fov = 0.4;
}

void processInput(GLFWwindow* window) {
	const float cameraSpeed = 0.025;
	const float rotationSpeed = 0.025f;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.cam_origin += normalize(camera.forward) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.cam_origin -= normalize(camera.forward) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.cam_origin -= glm::normalize(camera.right) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.cam_origin += glm::normalize(camera.right) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		camera.forward = glm::rotate(camera.forward, rotationSpeed, camera.up);
		camera.right = glm::cross(camera.forward, camera.up);
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		camera.forward = glm::rotate(camera.forward, -rotationSpeed, camera.up);
		camera.right = glm::cross(camera.forward, camera.up);
	}
}

int main() {
	glfwInit();
	
	//initial setup for camera data
	initialSetup();
	std::cout << "Camera Origin: ("
		<< camera.cam_origin.x << ", "
		<< camera.cam_origin.y << ", "
		<< camera.cam_origin.z << ")" << std::endl;

	bool firstMouse = true;
	float yaw = -90.0f;
	float pitch = 0.0f;
	float lastX = 400.0f;
	float lastY = 300.0f;
	
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Compute Shaders", NULL, NULL);

	if (!window) {
		std::cout << "Failed to create the GLFW window\n";
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(vSync);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize OpenGL context" << std::endl;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("C:\\Users\\ethan\\source\\repos\\Wormhole\\Wormhole\\shaders\\shader.vert", "C:\\Users\\ethan\\source\\repos\\Wormhole\\Wormhole\\shaders\\shader.frag");
	// Generates Shader object using shaders compute.glsl
	Shader computeProgram("C:\\Users\\ethan\\source\\repos\\Wormhole\\Wormhole\\shaders\\compute.glsl");

	GLuint shaderprogramID = shaderProgram.ID;

	GLuint VAO, VBO, EBO;
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferData(VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(EBO, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 5 * sizeof(GLfloat));
	glVertexArrayElementBuffer(VAO, EBO);

	GLuint screenTex;
	glCreateTextures(GL_TEXTURE_2D, 1, &screenTex);
	glTextureParameteri(screenTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(screenTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(screenTex, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindImageTexture(0, screenTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	unsigned int uboCameraBlock;
	glGenBuffers(1, &uboCameraBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, uboCameraBlock);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), &camera, GL_STATIC_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboCameraBlock);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);
		std::cout << "Camera Origin: ("
			<< camera.cam_origin.x << ", "
			<< camera.cam_origin.y << ", "
			<< camera.cam_origin.z << ")" << std::endl;

		computeProgram.Activate();
		glDispatchCompute(std::ceil(SCREEN_WIDTH / 8), std::ceil(SCREEN_HEIGHT / 4), 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		shaderProgram.Activate();

		glBindTextureUnit(0, screenTex);
		glUniform1i(glGetUniformLocation(shaderprogramID, "screen"), 0);

		glBindVertexArray(VAO);

		glNamedBufferSubData(uboCameraBlock, 0, sizeof(CameraData), &camera); //updates the CameraData struct

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	shaderProgram.Delete();
	computeProgram.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
}