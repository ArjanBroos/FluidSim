#include <glload/gl_3_3.h>
#include "framework.h"

GLuint program;					// Shader program
GLuint vao;						// Vertex array object
GLuint positionBufferObject;	// VBO with positions

const float vertexPositions[] = {
	0.75f, 0.75f, 0.0f, 1.0f,
	0.75f, -0.75f, 0.0f, 1.0f,
	-0.75f, -0.75f, 0.0f, 1.0f,
	-0.75f, 0.75f, 0.0f, 1.0f,
	0.75f, 0.75f, 0.0f, 1.0f,
	-0.75f, -0.75f, 0.0f, 1.0f
};

// Initializes the progam
void InitProgram() {
	std::vector<GLuint> shaders;
	shaders.push_back(Framework::LoadShader(GL_VERTEX_SHADER, "shader.vert"));
	shaders.push_back(Framework::LoadShader(GL_FRAGMENT_SHADER, "shader.frag"));

	program = Framework::CreateProgram(shaders);
}

void InitVertexBuffer() {
	glGenBuffers(1, &positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void InitVAO() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}

// Initializes our application
void init() {
	InitProgram();
	InitVertexBuffer();
	InitVAO();
}

// Renders the fluid
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	glutSwapBuffers();
}

// Simulates the fluid
void simulate() {
	glutPostRedisplay();
}

// Handles keyboard input
void keyboard(unsigned char key, int x, int y) {
	// Shut down program if ESC key is pressed
	if (key == 27) glutLeaveMainLoop();
}

// Handles reshaping of the window
void reshape(int width, int height) {

}