#include <glload/gl_3_3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "framework.h"
#include "fluidsimulator.h"

int windowWidth = 800;			// Width of the window
int windowHeight = 600;			// Height of the window
const float fovy = 60.f;		// Vertical field of view in degrees
const float zNear = 0.1f;		// Near plane
const float zFar = 1000.f;		// Far plane

GLuint program;					// Shader program
GLuint vao;						// Vertex array object
GLuint vbo;						// Vertex buffer object
GLuint ibo;						// Index buffer object
GLuint nbo;						// Normal buffer object

GLuint mvpMatrixUniform;		// Uniform ID for the MVP matrix
GLuint modelViewMatrixUniform;	// Uniform ID for the modelView matrix
GLuint normalMatrixUniform;		// Uniform ID for the normal matrix
GLuint viewMatrixUniform;		// Uniform ID for the view matrix
glm::mat4 modelMatrix;			// Matrix that transforms from model space to world space
glm::mat4 viewMatrix;			// Matrix that transforms from world space to camera space
glm::mat4 projectionMatrix;		// Matrix that transforms from camera space to clip space (does perspective projection)
glm::mat4 mvpMatrix;			// Product of the model, view and projection matrices
glm::mat4 modelViewMatrix;		// Product of the model and view matrices
glm::mat4 normalMatrix;			// Matrix used to transform normals

glm::vec3 cameraPosition;		// Position of our camera
glm::vec3 cameraLookAt;			// Position our camera is looking at

GLuint diffuseUniform;			// Uniform ID for the diffuse color
GLuint ambientUniform;			// Uniform ID for the ambient color
GLuint specularUniform;			// Uniform ID for the specular color
glm::vec4 diffuse;				// Diffuse color to render with
glm::vec4 ambient;				// Ambient color to render with
glm::vec4 specular;				// Specular color to render with

GLuint lightDirUniform;			// Uniform ID for the light direction
glm::vec3 lightDir;				// Direction of the incoming light

FluidSimulator fluidSimulator;	// The fluid simulator

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
const glm::vec3 cubeNormals[] = {
	glm::normalize(glm::vec3(-1.f,	1.f,	-1.f)),	// left top back
	glm::normalize(glm::vec3(1.f,	1.f,	-1.f)),	// right top back
	glm::normalize(glm::vec3(-1.f,	1.f,	1.f)),	// left top front
	glm::normalize(glm::vec3(1.f,	1.f,	1.f)),	// right top front
	glm::normalize(glm::vec3(-1.f,	-1.f,	-1.f)),	// left bottom back
	glm::normalize(glm::vec3(1.f,	-1.f,	-1.f)),	// right bottom back
	glm::normalize(glm::vec3(-1.f,	-1.f,	1.f)),	// left bottom front
	glm::normalize(glm::vec3(1.f,	-1.f,	1.f)),	// right bottom front
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

	modelViewMatrixUniform = glGetUniformLocation(program, "modelViewMatrix");
	normalMatrixUniform = glGetUniformLocation(program, "normalMatrix");
	mvpMatrixUniform = glGetUniformLocation(program, "mvpMatrix");
	viewMatrixUniform = glGetUniformLocation(program, "viewMatrix");
	diffuseUniform = glGetUniformLocation(program, "diffuseColor");
	ambientUniform = glGetUniformLocation(program, "ambientColor");
	specularUniform = glGetUniformLocation(program, "specularColor");
	lightDirUniform = glGetUniformLocation(program, "lightDir");

	glUseProgram(program);
	lightDir = glm::normalize(glm::vec3(1.f, -1.f, -2.f));
	glUniform3f(lightDirUniform, lightDir.x, lightDir.y, lightDir.z);
	glUseProgram(0);
}

// Sets up our vertex buffer
void InitVertexBuffer() {
	// Vertices
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &nbo);
	glBindBuffer(GL_ARRAY_BUFFER, nbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Sets up vertex array object
void InitVAO() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glBindBuffer(GL_ARRAY_BUFFER, nbo);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

// Sets up model, view and projection matrices
void InitMatrices() {
	glm::vec3 up(0.f, 1.f, 0.f);
	cameraPosition = glm::vec3(-20.f, 90.f, 150.f);
	cameraLookAt = glm::vec3(0.f, 0.f, 0.f);

	modelMatrix = glm::mat4(1.f);
	viewMatrix = glm::lookAt(cameraPosition, cameraLookAt, up);
	projectionMatrix = glm::perspective(fovy, (float)windowWidth / (float)windowHeight, zNear, zFar);
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

	// Do initialization for simulation
	for (float z = -50.f; z < 50.f; z += 5.f) {
		for (float y = -50.f; y < 50.f; y += 5.f) {
			for (float x = -50.f; x < 50.f; x += 5.f) {
				fluidSimulator.AddParticle(new Particle(x, y, z));
			}
		}
	}
}

// Renders the scene
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.f);

	glUseProgram(program);
	glBindVertexArray(vao);

	// Draw particles
	glUniform4f(diffuseUniform, 0.f, 0.f, 1.f, 1.f);
	glUniform4f(ambientUniform, 0.05f, 0.05f, 0.2f, 1.f);
	glUniform4f(specularUniform, 1.f, 1.f, 1.f, 1.f);
	viewMatrix = glm::lookAt(cameraPosition, cameraLookAt, glm::vec3(0.f, 1.f, 0.f));
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	std::vector<Particle*>& particles = fluidSimulator.GetParticles();
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		modelMatrix = glm::translate(glm::mat4(1.f), p->GetPosition());
		mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
		modelViewMatrix = viewMatrix * modelMatrix;
		glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
		glUniformMatrix4fv(normalMatrixUniform, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(GLshort), GL_UNSIGNED_SHORT, 0);
	}

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
	projectionMatrix = glm::perspective(fovy, (float)width / (float)height, zNear, zFar);
	glViewport(0, 0, width, height);
}