#include <glload/gl_3_3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <sstream>
#include "framework.h"
#include "fluidsimulator.h"

int windowWidth = 800;			// Width of the window
int windowHeight = 600;			// Height of the window
const float fovy = 60.f;		// Vertical field of view in degrees
const float zNear = 0.1f;		// Near plane
const float zFar = 1000.f;		// Far plane

struct BlockProgram {
	GLuint program;
	GLuint mvpMatrixUniform;		// Uniform ID for the MVP matrix
	GLuint modelViewMatrixUniform;	// Uniform ID for the modelView matrix
	GLuint normalMatrixUniform;		// Uniform ID for the normal matrix
	GLuint viewMatrixUniform;		// Uniform ID for the view matrix
	GLuint diffuseUniform;			// Uniform ID for the diffuse color
	GLuint ambientUniform;			// Uniform ID for the ambient color
	GLuint specularUniform;			// Uniform ID for the specular color
	GLuint lightDirUniform;			// Uniform ID for the light direction
	GLuint vao;						// Vertex array object
	GLuint vbo;						// Vertex buffer object
	GLuint ibo;						// Index buffer object
	GLuint nbo;						// Normal buffer object
};

struct SplatProgram {
	GLuint program;
	GLuint modelMatrixUniform;
	GLuint viewMatrixUniform;
	GLuint projectionMatrixUniform;
	GLuint normalMatrixUniform;
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
	GLuint nbo;
};

BlockProgram blockProgram;
SplatProgram splatProgram;

glm::mat4 modelMatrix;			// Matrix that transforms from model space to world space
glm::mat4 viewMatrix;			// Matrix that transforms from world space to camera space
glm::mat4 projectionMatrix;		// Matrix that transforms from camera space to clip space (does perspective projection)
glm::mat4 mvpMatrix;			// Product of the model, view and projection matrices
glm::mat4 modelViewMatrix;		// Product of the model and view matrices
glm::mat4 normalMatrix;			// Matrix used to transform normals

glm::vec3 cameraPosition;		// Position of our camera
glm::vec3 cameraLookAt;			// Position our camera is looking at

glm::vec4 diffuse;				// Diffuse color to render with
glm::vec4 ambient;				// Ambient color to render with
glm::vec4 specular;				// Specular color to render with

glm::vec3 lightDir;				// Direction of the incoming light

FluidSimulator fluidSimulator(	// The fluid simulator
	AABoundingBox(glm::vec3(0.f, 0.f, 0.f), 100.f));

int simTime = 0;				// Starting times of simulation and rendering,
int renderTime = 0;				// used for performance measurement
float fps = 0.f;				// Frames per second

void UpdateWindowTitle() {
	std::stringstream ss;
	ss << "FluidSim - Sim: " << simTime << "ms, Render: " << renderTime << "ms - FPS: " << floor(fps) << " wind: " << (fluidSimulator.isWind()?"Y":"N") << " gravity: " 
		<< (fluidSimulator.isGravity()?"Y":"N") << " surface tension: " << (fluidSimulator.isSurfaceTension()?"Y":"N");
	glutSetWindowTitle(ss.str().c_str());
}

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

// Hard code quad
const float quadVertices[] = {
	-0.5f,	0.5f,	0.f,	1.f,
	0.5f,	0.5f,	0.f,	1.f,
	0.5f,	-0.5f,	0.f,	1.f,
	-0.5f,	-0.5f,	0.f,	1.f,
};
const GLshort quadIndices[] = {
	0, 1, 2,	0, 2, 3,
};
const glm::vec3 quadNormals[] = {
	glm::vec3(0.f, 0.f, 1.f),
	glm::vec3(0.f, 0.f, 1.f),
	glm::vec3(0.f, 0.f, 1.f),
	glm::vec3(0.f, 0.f, 1.f),
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
void InitBlockProgram() {
	std::vector<GLuint> shaders;
	shaders.push_back(Framework::LoadShader(GL_VERTEX_SHADER, "blockShader.vert"));
	shaders.push_back(Framework::LoadShader(GL_FRAGMENT_SHADER, "blockShader.frag"));

	blockProgram.program = Framework::CreateProgram(shaders);

	blockProgram.modelViewMatrixUniform = glGetUniformLocation(blockProgram.program, "modelViewMatrix");
	blockProgram.normalMatrixUniform = glGetUniformLocation(blockProgram.program, "normalMatrix");
	blockProgram.mvpMatrixUniform = glGetUniformLocation(blockProgram.program, "mvpMatrix");
	blockProgram.viewMatrixUniform = glGetUniformLocation(blockProgram.program, "viewMatrix");
	blockProgram.diffuseUniform = glGetUniformLocation(blockProgram.program, "diffuseColor");
	blockProgram.ambientUniform = glGetUniformLocation(blockProgram.program, "ambientColor");
	blockProgram.specularUniform = glGetUniformLocation(blockProgram.program, "specularColor");
	blockProgram.lightDirUniform = glGetUniformLocation(blockProgram.program, "lightDir");

	glUseProgram(blockProgram.program);
	lightDir = glm::normalize(glm::vec3(1.f, -1.f, -2.f));
	glUniform3f(blockProgram.lightDirUniform, lightDir.x, lightDir.y, lightDir.z);
	glUseProgram(0);
}

void InitSplatProgram() {
	std::vector<GLuint> shaders;
	shaders.push_back(Framework::LoadShader(GL_VERTEX_SHADER, "splatShader.vert"));
	shaders.push_back(Framework::LoadShader(GL_FRAGMENT_SHADER, "splatShader.frag"));

	splatProgram.program = Framework::CreateProgram(shaders);

	splatProgram.modelMatrixUniform = glGetUniformLocation(splatProgram.program, "modelMatrix");
	splatProgram.viewMatrixUniform = glGetUniformLocation(splatProgram.program, "viewMatrix");
	splatProgram.projectionMatrixUniform = glGetUniformLocation(splatProgram.program, "projectionMatrix");
	splatProgram.normalMatrixUniform = glGetUniformLocation(splatProgram.program, "normalMatrix");
}

// Sets up our vertex buffer
void InitBlockVertexBuffer() {
	// Vertices
	glGenBuffers(1, &blockProgram.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, blockProgram.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	glGenBuffers(1, &blockProgram.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blockProgram.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &blockProgram.nbo);
	glBindBuffer(GL_ARRAY_BUFFER, blockProgram.nbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InitSplatVertexBuffer() {
	// Vertices
	glGenBuffers(1, &splatProgram.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, splatProgram.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	glGenBuffers(1, &splatProgram.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, splatProgram.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &splatProgram.nbo);
	glBindBuffer(GL_ARRAY_BUFFER, splatProgram.nbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadNormals), quadNormals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Sets up vertex array object
void InitBlockVAO() {
	glGenVertexArrays(1, &blockProgram.vao);
	glBindVertexArray(blockProgram.vao);

	glBindBuffer(GL_ARRAY_BUFFER, blockProgram.vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blockProgram.ibo);

	glBindBuffer(GL_ARRAY_BUFFER, blockProgram.nbo);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

// Sets up vertex array object
void InitSplatVAO() {
	glGenVertexArrays(1, &splatProgram.vao);
	glBindVertexArray(splatProgram.vao);

	glBindBuffer(GL_ARRAY_BUFFER, splatProgram.vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, splatProgram.ibo);

	glBindBuffer(GL_ARRAY_BUFFER, splatProgram.nbo);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

// Sets up model, view and projection matrices
void InitMatrices() {
	glm::vec3 up(0.f, 1.f, 0.f);
	cameraPosition = glm::vec3(-20.f, 50.f, 130.f);
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

void AddParticles() {
	for (float z = -50.f; z < 40.f; z += 6.f) {
		for (float y = 0.f; y < 40.f; y += 6.f) {
			for (float x = 0.f; x < 40.f; x += 6.f) {
				glm::vec3 position(x, y, z);
				fluidSimulator.AddParticle(new Particle(position));
			}
		}
	}
}

void AddBodies() {
	fluidSimulator.AddBody(new sphere(glm::vec3(0.f, 70.f, 0.f),40.0));
}

// Initializes our application
void init() {
	InitOpenGL();
	InitBlockProgram();
	InitBlockVertexBuffer();
	InitBlockVAO();
	InitSplatProgram();
	InitSplatVertexBuffer();
	InitSplatVAO();
	InitMatrices();

	// Do initialization for simulation
	AddParticles();
	AddBodies();
}

void DisplayBlocks() {
	glUseProgram(blockProgram.program);
	glBindVertexArray(blockProgram.vao);

	// Draw particles
	glUniform4f(blockProgram.diffuseUniform, 0.f, 0.f, 1.f, 1.f);
	glUniform4f(blockProgram.ambientUniform, 0.05f, 0.05f, 0.2f, 1.f);
	glUniform4f(blockProgram.specularUniform, 1.f, 1.f, 1.f, 1.f);
	viewMatrix = glm::lookAt(cameraPosition, cameraLookAt, glm::vec3(0.f, 1.f, 0.f));
	glUniformMatrix4fv(blockProgram.viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	const float particleScale = 3.f;
	std::vector<Particle*>& particles = fluidSimulator.GetParticles();
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		modelMatrix = glm::scale(glm::mat4(1.f), glm::vec3(particleScale, particleScale, particleScale));
		modelMatrix = glm::translate(modelMatrix, p->position / particleScale);
		mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
		glUniformMatrix4fv(blockProgram.mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
		modelViewMatrix = viewMatrix * modelMatrix;
		glUniformMatrix4fv(blockProgram.modelViewMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
		normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
		glUniformMatrix4fv(blockProgram.normalMatrixUniform, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(GLshort), GL_UNSIGNED_SHORT, 0);
	}


	std::vector<body*>& bodies = fluidSimulator.GetBodies();
	for (auto bi = bodies.begin(); bi != bodies.end(); bi++) {
		body* b = *bi;
		b->draw(blockProgram.mvpMatrixUniform,
			blockProgram.modelViewMatrixUniform,
			blockProgram.normalMatrixUniform,
			modelMatrix,
			viewMatrix,
			projectionMatrix,
			mvpMatrix,
			modelViewMatrix,
			normalMatrix);
	}
	
	glBindVertexArray(0);
	glUseProgram(0);
}

void DisplaySplats() {
	glUseProgram(splatProgram.program);
	glBindVertexArray(splatProgram.vao);

	// Set up view matrix
	viewMatrix = glm::lookAt(cameraPosition, cameraLookAt, glm::vec3(0.f, 1.f, 0.f));
	glUniformMatrix4fv(splatProgram.viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	const float particleScale = 3.f;
	std::vector<Particle*>& particles = fluidSimulator.GetParticles();
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		modelMatrix = glm::scale(glm::mat4(1.f), glm::vec3(particleScale));
		modelMatrix = glm::translate(modelMatrix, p->position / particleScale);
		normalMatrix = glm::transpose(glm::inverse(viewMatrix * modelMatrix));
		glUniformMatrix4fv(splatProgram.modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(splatProgram.normalMatrixUniform, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		glDrawElements(GL_TRIANGLES, sizeof(quadIndices) / sizeof(GLshort), GL_UNSIGNED_SHORT, 0);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

// Renders the scene
void display() {
	int startRender = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.f);

	DisplaySplats();
	//DisplayBlocks();

	glutSwapBuffers();
	renderTime = glutGet(GLUT_ELAPSED_TIME) - startRender;
	UpdateWindowTitle();
}

// Simulates the fluid
void simulate() {
	int startSim = glutGet(GLUT_ELAPSED_TIME);

	// Use time required for previous frame as timestep
	float dt = (float)(simTime + renderTime) / 1000.f;
	fps = 1.f / dt;

	fluidSimulator.ExplicitEulerStep(0.1f);

	// Start drawing again
	glutPostRedisplay();
	simTime = glutGet(GLUT_ELAPSED_TIME) - startSim;
}

// Handles keyboard input
void keyboard(unsigned char key, int x, int y) {
	// Shut down program if ESC key is pressed
	if (key == 27) glutLeaveMainLoop();
	// Reset simulation if Space key is pressed
	if (key == ' ') { fluidSimulator.Clear(); AddParticles(); AddBodies(); }
	// Toggle gravity force with G key
	if (key == 'g') { fluidSimulator.ToggleFluidGravity(); }
	// Toggle gravity force with G key
	if (key == 'b') { fluidSimulator.ToggleBodyGravity(); }
	// Toggle wind force with W key
	if (key == 'w') { fluidSimulator.ToggleWind(); }
	if (key == 'i'){ fluidSimulator.movingBody->position -= glm::vec3(0.f, 0.f, 1.f); }
	if (key == 'k'){ fluidSimulator.movingBody->position += glm::vec3(0.f, 0.f, 1.f); }
	// Toggle surface tension force with S key
	if (key == 's') { fluidSimulator.ToggleSurfaceTension(); }
}

// Handles reshaping of the window
void reshape(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	projectionMatrix = glm::perspective(fovy, (float)width / (float)height, zNear, zFar);
	glUniformMatrix4fv(splatProgram.projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glViewport(0, 0, width, height);
}