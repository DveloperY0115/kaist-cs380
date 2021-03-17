#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <random>
#include <cassert>

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

	virtual void BindVBOs() {
		// Do nothing
	}

	virtual void DrawObj(const ShaderState& curSS) {
		// Do nothing
	}

private:
	GlBufferObject posVBO, colVBO;
	GLfloat* vert;
	GLfloat* col;
};

class RandomTriangles : public Geometry {

public:

	RandomTriangles(unsigned int num_verts) {
		num_vertices = num_verts;
		vertices = new GLfloat[3 * num_verts];
		colors = new GLfloat[3 * num_verts];
		GeneratePoints2D();
		BindVBOs();
	}

	void BindVBOs() final {
		// initialize pos buffer
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);    // bind coordinate buffer
		glBufferData(GL_ARRAY_BUFFER,    // transfer data to GPU
			3 * num_vertices * sizeof(GLfloat),
			vertices,
			GL_STATIC_DRAW);
		checkGlErrors();

		// initialize color buffer
		glBindBuffer(GL_ARRAY_BUFFER, colVBO);    // bind color buffer
		glBufferData(GL_ARRAY_BUFFER,    // transfer data to GPU
			3 * num_vertices * sizeof(GLfloat),
			colors,
			GL_STATIC_DRAW);
		checkGlErrors();
	}

	void GeneratePoints2D() {
		std::array<GLfloat, 3> coord = {0};
		int i = 0;
		for (i; i < num_vertices; ++i) {
			GenerateRandomPoint2D(coord, -1.0, 1.0);
			vertices[3 * i] = coord[0];
			vertices[3 * i + 1] = coord[1];
			vertices[3 * i + 2] = coord[2];    
		}

		std::array<GLfloat, 3> color;
		for (i = 0; i < num_vertices; ++i) {
			GenerateRandomColor(color);
			colors[3 * i] = color[0];
			colors[3 * i + 1] = color[1];
			colors[3 * i + 2] = color[2];
		}
	}

	 void GenerateRandomPoint2D(std::array<GLfloat, 3>& arr, float start, float end) {
		 // input must be three dimensional
		 assert(arr.size() == 3);

		 // set random seed and distribution
		 std::random_device rd;
		 std::mt19937 gen(rd());
		 std::uniform_real_distribution<> distribution(start, end);

		 // generate random point
		 arr[0] = distribution(rd);
		 arr[1] = distribution(rd);
		 arr[2] = 0.0;
	}

	 void GenerateRandomColor(std::array<GLfloat, 3>& arr) {
		 // input must be three dimensional
		 assert(arr.size() == 3);

		 // set random seed and distribution
		 std::random_device rd;
		 std::mt19937 gen(rd());
		 std::uniform_real_distribution<> distribution(0.0, 1.0);

		 // generate random point
		 arr[0] = distribution(rd);
		 arr[1] = distribution(rd);
		 arr[2] = distribution(rd);
	 }

	 virtual void DrawObj(const ShaderState& curSS) final {
		 safe_glEnableVertexAttribArray(curSS.h_aPos);
		 safe_glEnableVertexAttribArray(curSS.h_aColor);

		 glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		 safe_glVertexAttribPointer(curSS.h_aPos,
			 3, GL_FLOAT, GL_FALSE, 0, 0);

		 glBindBuffer(GL_ARRAY_BUFFER, colVBO);
		 safe_glVertexAttribPointer(curSS.h_aColor,
			 3, GL_FLOAT, GL_FALSE, 0, 0);

		 glDrawArrays(GL_TRIANGLES, 0, num_vertices);

		 safe_glDisableVertexAttribArray(curSS.h_aPos);
		 safe_glDisableVertexAttribArray(curSS.h_aColor);
	 }

	 void Describe() {
		 // Print out information about the instance

		 std::cout << "========================\n";

		 std::cout << "Vertices: \n";

		 for (int i = 0; i < 3 * num_vertices; ++i) {
			 std::cout << vertices[i];
			 if (i % 3 == 0)
				 std::cout << "\n";
		 }

		 std::cout << "========================\n";

		 std::cout << "Colors(RGB): \n";
		 
		 for (int i = 0; i < 3 * num_vertices; ++i) {
			 std::cout << colors[i];
			 if (i % 3 == 0)
				 std::cout << "\n";
		 }
	 }

	 GLfloat* GetVertices() {
		 assert(vertices != NULL);
		 return vertices;
	 }

	 GLfloat* GetColors() {
		 assert(colors != NULL);
		 return colors;
	 }

private:
	GlBufferObject posVBO, colVBO;
	unsigned int num_vertices;
	GLfloat* vertices;
	GLfloat* colors;
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
static std::shared_ptr<RandomTriangles> g_random_triangle;

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
	// g_simple.reset(new SimpleGeometry());
	g_random_triangle.reset(new RandomTriangles(100));

	// print out result
	std::cout << "Geometry Initialized!\n";
	g_random_triangle->Describe();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const ShaderState& curSS = *g_ShaderStates[0];
	glUseProgram(curSS.programHandle);
	
	g_random_triangle->DrawObj(curSS);

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