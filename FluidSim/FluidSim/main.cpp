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
bool paused = false;

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
	GLuint projectionMatrixUniform;
	GLuint cameraLightDirUniform;	// Uniform ID for the light direction in camera space
	GLuint vao;						// Vertex array object
	GLuint cameraPositionUniform;	// Uniform ID for the sphere's center in camera space
	GLuint sphereRadiusUniform;		// Uniform ID for the sphere's radius
	GLuint baseColorUniform;
	GLuint opaquenessUniform;
};

struct BasicProgram {
	GLuint program;
	GLuint mvpMatrixUniform;
	GLuint colorUniform;
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
};

BlockProgram blockProgram;
SplatProgram splatProgram;
BasicProgram basicProgram;

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
const GLshort cubeSideIndices[] = {
	0, 1,		2, 3,
	4, 5,		6, 7,
	0, 2,		1, 3,
	4, 6,		5, 7,
	0, 4,		1, 5,
	2, 6,		3, 7,
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
	glClearDepth(1.f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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

// Initializes the progam
void InitSplatProgram() {
	std::vector<GLuint> shaders;
	shaders.push_back(Framework::LoadShader(GL_VERTEX_SHADER, "splatShader.vert"));
	shaders.push_back(Framework::LoadShader(GL_FRAGMENT_SHADER, "splatShader.frag"));

	splatProgram.program = Framework::CreateProgram(shaders);

	splatProgram.cameraLightDirUniform = glGetUniformLocation(splatProgram.program, "cameraLightDir");
	splatProgram.projectionMatrixUniform = glGetUniformLocation(splatProgram.program, "projectionMatrix");
	splatProgram.cameraPositionUniform = glGetUniformLocation(splatProgram.program, "cameraSpherePos");
	splatProgram.sphereRadiusUniform = glGetUniformLocation(splatProgram.program, "sphereRadius");
	splatProgram.baseColorUniform = glGetUniformLocation(splatProgram.program, "baseColor");
	splatProgram.opaquenessUniform = glGetUniformLocation(splatProgram.opaquenessUniform, "opaqueness");

	lightDir = glm::normalize(glm::vec3(1.f, -3.f, -2.f));
}
// Initializes the progam
void InitBasicProgram() {
	std::vector<GLuint> shaders;
	shaders.push_back(Framework::LoadShader(GL_VERTEX_SHADER, "basicShader.vert"));
	shaders.push_back(Framework::LoadShader(GL_FRAGMENT_SHADER, "basicShader.frag"));

	basicProgram.program = Framework::CreateProgram(shaders);

	basicProgram.mvpMatrixUniform = glGetUniformLocation(basicProgram.program, "mvpMatrix");
	basicProgram.colorUniform = glGetUniformLocation(basicProgram.program, "color");
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
}

void InitBasicVertexBuffer() {
	// Vertices
	glGenBuffers(1, &basicProgram.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, basicProgram.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Indices
	glGenBuffers(1, &basicProgram.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, basicProgram.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeSideIndices), cubeSideIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
}

// Sets up vertex array object
void InitBasicVAO() {
	glGenVertexArrays(1, &basicProgram.vao);
	glBindVertexArray(basicProgram.vao);

	glBindBuffer(GL_ARRAY_BUFFER, basicProgram.vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, basicProgram.ibo);

	glBindVertexArray(0);
}


// Sets up model, view and projection matrices
void InitMatrices() {
	glm::vec3 up(0.f, 1.f, 0.f);
	cameraPosition = glm::vec3(-40.f, 80.f, 130.f);
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
	fluidSimulator.AddBody(new sphere(glm::vec3(0.f, 70.f, 0.f),30.0, 5.f));

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
	InitBasicProgram();
	InitBasicVertexBuffer();
	InitBasicVAO();
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
	
	glBindVertexArray(0);
	glUseProgram(0);
}

void DisplaySplats() {
	glUseProgram(splatProgram.program);
	glBindVertexArray(splatProgram.vao);

	const float sphereRadius = 8.f;

	glUniform4f(ambientUniform, 0.05f, 0.2f, 0.05f, 1.f);
	std::vector<body*>& bodies = fluidSimulator.GetBodies();
	for (auto bi = bodies.begin(); bi != bodies.end(); bi++) {
		body* b = *bi;
		b->draw(mvpMatrixUniform,
			modelViewMatrixUniform,
			normalMatrixUniform,
			modelMatrix,
			viewMatrix,
			projectionMatrix,
			mvpMatrix,
			modelViewMatrix,
			normalMatrix);
	// Draw particles
	viewMatrix = glm::lookAt(cameraPosition, cameraLookAt, glm::vec3(0.f, 1.f, 0.f));
	glm::vec3 cameraLightDir = glm::vec3(viewMatrix * glm::vec4(lightDir, 0.f));

	std::vector<Particle*>& particles = fluidSimulator.GetParticles();
	for (auto pi = particles.begin(); pi != particles.end(); pi++) {
		Particle* p = *pi;
		
		glm::vec3 cameraPosition = glm::vec3(viewMatrix * glm::vec4(p->position, 1.f));

		glUniform3fv(splatProgram.cameraLightDirUniform, 1, glm::value_ptr(cameraLightDir));
		glUniform3fv(splatProgram.cameraPositionUniform, 1, glm::value_ptr(cameraPosition)); 
		glUniform1f(splatProgram.sphereRadiusUniform, sphereRadius);
		glUniform3f(splatProgram.baseColorUniform, 0.f, 0.06f, 1.f);
		glUniform1f(splatProgram.opaquenessUniform, 0.5f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}
	
	glBindVertexArray(0);
	glUseProgram(0);
}

void DisplayBoundingBox() {
	glUseProgram(basicProgram.program);
	glBindVertexArray(basicProgram.vao);

	viewMatrix = glm::lookAt(cameraPosition, cameraLookAt, glm::vec3(0.f, 1.f, 0.f));

	const AABoundingBox& bb = fluidSimulator.GetBoundingBox();
	modelMatrix = glm::scale(glm::mat4(1.f), glm::vec3(bb.size));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(bb.center / bb.size));

	glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(basicProgram.mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
	glUniform4f(basicProgram.colorUniform, 1.f, 1.f, 1.f, 1.f);
	glDrawElements(GL_LINES, sizeof(cubeSideIndices) / sizeof(GLshort), GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

void DisplayBody() {
	glUseProgram(splatProgram.program);
	glBindVertexArray(splatProgram.vao);

	viewMatrix = glm::lookAt(cameraPosition, cameraLookAt, glm::vec3(0.f, 1.f, 0.f));
	glm::vec3 cameraLightDir = glm::vec3(viewMatrix * glm::vec4(lightDir, 0.f));

	const Sphere* sphere = (Sphere*)fluidSimulator.movingBody;
	glm::vec3 cameraPosition = glm::vec3(viewMatrix * glm::vec4(sphere->position, 1.f));

	glUniform3fv(splatProgram.cameraLightDirUniform, 1, glm::value_ptr(cameraLightDir));
	glUniform3fv(splatProgram.cameraPositionUniform, 1, glm::value_ptr(cameraPosition)); 
	glUniform1f(splatProgram.sphereRadiusUniform, sphere->size);
	glUniform3f(splatProgram.baseColorUniform, 0.f, 1.f, 0.f);
	glUniform1f(splatProgram.opaquenessUniform, 1.f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
}

// Renders the scene
void display() {
	int startRender = glutGet(GLUT_ELAPSED_TIME);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//DisplayBlocks();
	DisplayBody();
	DisplayBoundingBox();
	DisplaySplats();

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

	if (!paused)
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

	if (key == 'i'){ fluidSimulator.movingBody->center -= glm::vec3(0.f, 0.f, 1.f); }
	if (key == 'k'){ fluidSimulator.movingBody->center += glm::vec3(0.f, 0.f, 1.f); }

	// Toggle surface tension force with S key
	if (key == 's') { fluidSimulator.ToggleSurfaceTension(); }
	// Pause simulation with P key
	if (key == 'p') { paused = !paused; }
}

// Handles reshaping of the window
void reshape(int width, int height) {
	windowWidth = width;
	windowHeight = height;
	projectionMatrix = glm::perspective(fovy, (float)width / (float)height, zNear, zFar);
	glUseProgram(splatProgram.program);
	glUniformMatrix4fv(splatProgram.projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUseProgram(0);
	glViewport(0, 0, width, height);
}