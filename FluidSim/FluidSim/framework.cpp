//Copyright (C) 2010-2012 by Jason L. McKesson
//This file is licensed under the MIT License.
#include <algorithm>
#include <fstream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <string.h>
#include <glload/gl_3_3.h>
#include <glload/gl_load.hpp>
#include <glutil/Shader.h>
#include "framework.h"

namespace Framework
{
	GLuint LoadShader(GLenum eShaderType, const std::string &strShaderFilename)
	{
		std::string strFilename = FindFileOrThrow(strShaderFilename);
		std::ifstream shaderFile(strFilename.c_str());
		std::stringstream shaderData;
		shaderData << shaderFile.rdbuf();
		shaderFile.close();

		try
		{
			return glutil::CompileShader(eShaderType, shaderData.str());
		}
		catch(std::exception &e)
		{
			fprintf(stderr, "%s\n", e.what());
			throw;
		}
	}

	GLuint CreateProgram(const std::vector<GLuint> &shaderList)
	{
		try
		{
			GLuint prog = glutil::LinkProgram(shaderList);
			std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
			return prog;
		}
		catch(std::exception &e)
		{
			std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
			fprintf(stderr, "%s\n", e.what());
			throw;
		}
	}

	float DegToRad(float fAngDeg)
	{
		const float fDegToRad = 3.14159f * 2.0f / 360.0f;
		return fAngDeg * fDegToRad;
	}

	std::string FindFileOrThrow( const std::string &strBasename )
	{
		std::string strFilename = "shaders\\" + strBasename;
		std::ifstream testFile(strFilename.c_str());
		if(testFile.is_open())
			return strFilename;

		
		strFilename = "..\\shaders\\" + strBasename;
		testFile.open(strFilename.c_str());
		if(testFile.is_open())
			return strFilename;

		throw std::runtime_error("Could not find the file " + strBasename);
	}
}


void init();
void display();
void simulate();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);

unsigned int defaults(unsigned int displayMode, int &width, int &height);

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	int width = 1024;
	int height = 768;
	unsigned int displayMode = GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL;
	displayMode = defaults(displayMode, width, height);

	glutInitDisplayMode (displayMode);
	glutInitContextVersion (3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitWindowSize (width, height); 
	glutInitWindowPosition (300, 200);
	int window = glutCreateWindow("FluidSim");

	glload::LoadFunctions();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	if(!glload::IsVersionGEQ(3, 3))
	{
		printf("Your OpenGL version is %i, %i. You must have at least OpenGL 3.3 to run this tutorial.\n",
			glload::GetMajorVersion(), glload::GetMinorVersion());
		glutDestroyWindow(window);
		return 0;
	}

	init();

	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(simulate);
	glutMainLoop();
	return 0;
}
