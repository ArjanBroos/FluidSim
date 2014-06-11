#include <glload/gl_3_3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "framework.h"

int windowWidth = 800;			// Width of the window
int windowHeight = 600;			// Height of the window
const float fovy = 60.f;		// Vertical field of view in degrees
const float zNear = 0.1f;		// Near plane
const float zFar = 1000.f;		// Far plane

GLuint program;					// Shader program
GLuint vao;						// Vertex array object
GLuint vbo;						// Vertex buffer object
GLuint ibo;						// Index buffer object

GLuint mvpUniform;				// Uniform ID for the MVP matrix;
glm::mat4 model;				// Matrix that transforms from model space to world space
glm::mat4 view;					// Matrix that transforms from world space to camera space
glm::mat4 projection;			// Matrix that transforms from camera space to clip space (does perspective projection)
glm::mat4 mvp;					// Product of the model, view and projection matrices

glm::vec3 cameraPosition;		// Position of our camera
glm::vec3 cameraLookAt;			// Position our camera is looking at

// Hard code cube model
const float cubeVertices[] = {
	-0.5f,	0.5f,	-0.5f,	1.f,	// left top back		0
	0.5f,	0.5f,	-0.5f,	1.f,	// right top back		1
	-0.5f,	0.5f,	0.5f,	1.f,	// left top front		2
	0.5f,	0.5f,	0.5f,	1.f,	// right top front		3
	-0.5f,	-0.5f,	-0.5f,	1.f,	// left bottom back		4
	0.5f,	-0.5f,	-0.5f,	1.f,	// right bottom back	5
	-0.5f,	-0.5f,	0.5f,	1.f,	// left bottom front	6
	0.5f,	-0.5f,	0.5f,	1.f,	// right bottom front	7
};
const GLshort cubeIndices[] = {
	2, 3, 6,	3, 7, 6,	// Front face
	0, 4, 5,	1, 0, 5,	// Back face
	0, 1, 2,	1, 3, 2,	// Top face
	6, 7, 5,	6, 5, 4,	// Bottom face
	0, 2, 4,	2, 6, 4,	// Left face
	3, 1, 5,	3, 5, 7,	// Right face
};

// Sets some OpenGL states
void InitOpenGL() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
}

// Initializes the progam
void InitProgram() {
	std::vector<GLuint> shaders;
	shaders.push_back(Framework::LoadShader(GL_VERTEX_SHADER, "shader.vert"));
	shaders.push_back(Framework::LoadShader(GL_FRAGMENT_SHADER, "shader.frag"));

	program = Framework::CreateProgram(shaders);

	mvpUniform = glGetUniformLocation(program, "mvp");
}

// Sets up our vertex buffer
void InitVertexBuffer() {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Sets up vertex array object
void InitVAO() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glBindVertexArray(0);
}

// Sets up model, view and projection matrices
void InitMatrices() {
	glm::vec3 up(0.f, 1.f, 0.f);
	cameraPosition = glm::vec3(1.f, 1.f, 2.f);
	cameraLookAt = glm::vec3(0.f, 0.f, 0.f);

	model = glm::mat4(1.f);
	view = glm::lookAt(cameraPosition, cameraLookAt, up);
	projection = glm::perspective(fovy, (float)windowWidth / (float)windowHeight, zNear, zFar);
	mvp = projection * view * model;
}

// Specify display mode and window size for the framework
unsigned int defaults(unsigned int displayMode, int& width, int& height) {
	width = windowWidth;
	height = windowHeight;
	return GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
}

// Initializes our application
void init() {
	InitOpenGL();
	InitProgram();
	InitVertexBuffer();
	InitVAO();
	InitMatrices();
}

// Renders the scene
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.f);

	glUseProgram(program);
	glBindVertexArray(vao);

	mvp = projection * view * model;
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, glm::value_ptr(mvp));

	glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(GLshort), GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	glUseProgram(0);

	glutSwapBuffers();
}

// Simulates the fluid
void simulate() {
	// TODO: simulation

	// Start drawing again
	glutPostRedisplay();
}

// Handles keyboard input
void keyboard(unsigned char key, int x, int y) {
	// Shut down program if ESC key is pressed
	if (key == 27) glutLeaveMainLoop();
}

// Handles reshaping of the window
void reshape(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	projection = glm::perspective(fovy, (float)width / (float)height, zNear, zFar);
	glViewport(0, 0, width, height);
}