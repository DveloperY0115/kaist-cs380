#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GL/glut.h>

#include "ppm.h"
#include "glsupport.h"

// forward declaration
void initGLUT(int argc, char** argv);
void keyboardCallback(unsigned char key, int x, int y);;
void display();

// global variables
static int winID = 0;
static int g_width = 512;
static int g_height = 512;

typedef struct ShaderState {
	
	// Handle for shader program
	GlProgram programHandle;
	
	// Handles to attribute variables
	
	GLuint h_aPos;
	GLuint h_aColor;

	//! Constructor
	//! Initialize shader program with given VS, FS sources
	ShaderState(const char* vsfn, const char* fsfn) {
		
		// read VS, FS source, compile them, and link them
		readAndCompileShader(programHandle, vsfn, fsfn);
		std::cerr << "Built OpenGL shader successfully\n";

		h_aPos = safe_glGetAttribLocation(programHandle, "aPos");
		h_aColor = safe_glGetAttribLocation(programHandle, "aColor");
		glBindFragDataLocation(programHandle, 0, "FragColor");
	}

} ShaderState;

// To render an image on OpenGL, we need a pair of VS and FS
static const int g_numShaders = 1;

// Specify locations of GLSL source files in disk
static const char* const g_shaderFiles[g_numShaders][2] = {
	// GLSL 1.3 shaders
  {"./shaders/vs.vshader", "./shaders/fs.fshader"}
};    // GLSL 1.3

// Vector holding pointers to ShaderState structs
static std::vector<std::shared_ptr<ShaderState>> g_ShaderStates;

//! Base class for Geometry
class Geometry {

public:
	Geometry() {
		// Constructor of Base class
		// Do nothing
	}

	virtual void BindVBOs(unsigned int num_verts) {
		// initialize pos buffer
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);    // bind coordinate buffer
		glBufferData(GL_ARRAY_BUFFER,    // transfer data to GPU
			num_verts * sizeof(GLfloat),
			vert,
			GL_STATIC_DRAW);
		checkGlErrors();

		// initialize color buffer
		glBindBuffer(GL_ARRAY_BUFFER, colVBO);    // bind color buffer
		glBufferData(GL_ARRAY_BUFFER,    // transfer data to GPU
			num_verts * sizeof(GLfloat),
			col,
			GL_STATIC_DRAW);
		checkGlErrors();
	}

	virtual void drawObjs(ShaderState& curSS) {
		// Do nothing
	}

private:
	GlBufferObject posVBO, colVBO;
	GLfloat* vert;
	GLfloat* col;
};
typedef struct SimpleGeometry {
	GlBufferObject posVBO, colVBO;

	SimpleGeometry() {
		// triangle at the center of the screen
		static GLfloat vertices[] = {
			0.0f, 0.5f, 0.0f,
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f
		};

		// colors of each vertice
		static GLfloat colors[]{
			1, 0, 0,
			0, 1, 0,
			0, 0, 1
		};

		// initialize pos buffer
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);    // bind coordinate buffer
		glBufferData(GL_ARRAY_BUFFER,    // transfer data to GPU
			9 * sizeof(GLfloat), 
			vertices, 
			GL_STATIC_DRAW);
		checkGlErrors();

		// initialize color buffer
		glBindBuffer(GL_ARRAY_BUFFER, colVBO);    // bind color buffer
		glBufferData(GL_ARRAY_BUFFER,    // transfer data to GPU
			9 * sizeof(GLfloat),
			colors,
			GL_STATIC_DRAW);
		checkGlErrors();
	}

	void drawObj(const ShaderState& curSS) {

		safe_glEnableVertexAttribArray(curSS.h_aPos);
		safe_glEnableVertexAttribArray(curSS.h_aColor);

		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		safe_glVertexAttribPointer(curSS.h_aPos,
				3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, colVBO);
		safe_glVertexAttribPointer(curSS.h_aColor,
				3, GL_FLOAT, GL_FALSE, 0, 0);
	    
		glDrawArrays(GL_TRIANGLES, 0, 3);

		safe_glDisableVertexAttribArray(curSS.h_aPos);
		safe_glDisableVertexAttribArray(curSS.h_aColor);
	}
} SimpleGeometry;

static std::shared_ptr<SimpleGeometry> g_simple;

void initGLUT(int argc, char** argv) {	

	// Initialize GLUT and window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(g_width, g_height);
	winID = glutCreateWindow("Test");

	// Register callback functions
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboardCallback);
}

static void initGLState() {
	glClearColor(128. / 255, 200. / 255, 1, 0);    // This is why our background color is sky blue!!
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
}

static void initShaders() {
	g_ShaderStates.resize(g_numShaders);
	for (int i = 0; i < g_numShaders; ++i) {
			g_ShaderStates[i].reset(new ShaderState(g_shaderFiles[i][0], g_shaderFiles[i][1]));
	}
}

static void initGeometry() {
	g_simple.reset(new SimpleGeometry());
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	const ShaderState& curSS = *g_ShaderStates[0];
	glUseProgram(curSS.programHandle);

	g_simple->drawObj(curSS);
	
	glutSwapBuffers();
	checkGlErrors();
}

void keyboardCallback(unsigned char key, int x, int y) {

	switch (key) {
	case 'q':
		// quit program
		std::cout << "Recieved 'q'! Terminating... \n";
		glutDestroyWindow(winID);
		exit(EXIT_SUCCESS);
	}
}

int main(int argc, char** argv) {

	// initialize GLUT
	initGLUT(argc, argv);    // REMARK! Always initialize GLUT before initializing GLEW

	// initialize GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		printf("GLEW init failed: %s!n", glewGetErrorString(err));
		exit(1);
	}
	else
	{
		printf("GLEW init success!\n");
	}

	initGLState();
	initShaders();
	initGeometry();
	glutMainLoop();   // block here
	return 0;
}