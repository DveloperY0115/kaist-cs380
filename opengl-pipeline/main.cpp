#include <iostream>
#include <string>
#include <array>
#include <list>
#include <algorithm>
#include <memory>
#include <random>
#include <cassert>

#include <GL/glew.h>
#include <GL/glut.h>

#include "ppm.h"
#include "glsupport.h"

// forward declaration
void initGLUT(int argc, char** argv);
void keyboardCallback(unsigned char key, int x, int y);
void timerCallback(int value);
void display();

// global variables
static int winID = 0;
static int g_width = 512;
static int g_height = 512;
const static int LIFE_CNT = 10;

typedef struct ShaderState {

	// Handle for shader program
	GlProgram programHandle;

	// Handles to attribute variables

	GLuint h_aPos;
	GLuint h_aColor;
	GLuint h_aOpacity;

	//! Constructor
	//! Initialize shader program with given VS, FS sources
	ShaderState(const char* vsfn, const char* fsfn) {

		// read VS, FS source, compile them, and link them
		readAndCompileShader(programHandle, vsfn, fsfn);
		std::cerr << "Built OpenGL shader successfully\n";

		// retrieve handles for vertex attributes
		h_aPos = safe_glGetAttribLocation(programHandle, "aPos");
		h_aColor = safe_glGetAttribLocation(programHandle, "aColor");
		h_aOpacity = safe_glGetAttribLocation(programHandle, "aOpacity");

		// bind output to GLSL variable "FragColor"
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
		colors = new GLfloat[4 * num_verts];
		life_cnt = 0;
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

		std::array<GLfloat, 4> color;
		for (i = 0; i < num_vertices; ++i) {
			GenerateRandomColor(color);
			colors[4 * i] = color[0];
			colors[4 * i + 1] = color[1];
			colors[4 * i + 2] = color[2];
			colors[4 * i + 3] = color[3];
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

	 void GenerateRandomColor(std::array<GLfloat, 4>& arr) {
		 // input must be three dimensional
		 assert(arr.size() == 4);

		 // set random seed and distribution
		 std::random_device rd;
		 std::mt19937 gen(rd());
		 std::uniform_real_distribution<> distribution(0.0, 1.0);

		 // generate random point
		 arr[0] = distribution(rd);
		 arr[1] = distribution(rd);
		 arr[2] = distribution(rd);
		 arr[3] = 1.0;    // initial opacity is 1
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

	 unsigned int GetLife() {
		 return life_cnt;
	 }

	 void IncreaseLifeCount() {
		 life_cnt++;
	 }

private:
	GlBufferObject posVBO, colVBO;
	unsigned int num_vertices;
	GLuint life_cnt;
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
// static std::shared_ptr<RandomTriangles> g_random_triangle;

// queue of pointers of RandomTriangle instance
static std::list<std::shared_ptr<RandomTriangles>> g_random_triangles;

void initGLUT(int argc, char** argv) {

	// Initialize GLUT and window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(g_width, g_height);
	winID = glutCreateWindow("Test");

	// Register callback functions
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboardCallback);
	// glutTimerFunc(1, timerCallback, 1);    // 1 is dummy a variable
}

static void initGLState() {
	glClearColor(128. / 255, 200. / 255, 1, 0);    // This is why our background color is sky blue!!
	// glClearColor(1.0, 1.0, 1.0, 0);    // This is why our background color is sky blue!!
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
	std::shared_ptr<RandomTriangles> g_random_triangle; 
	// print out result
	// std::cout << "Geometry Initialized!\n";
	// g_random_triangle->Describe();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ShaderState& curSS = *g_ShaderStates[0];
	glUseProgram(curSS.programHandle);
	
	// reduce life count by one
	if (g_random_triangles.size() != 0) {
		for (auto& curr_geometry : g_random_triangles)
			curr_geometry->IncreaseLifeCount();
	}
	
	if (g_random_triangles.size() != 0) {
		if (g_random_triangles.front()->GetLife() == LIFE_CNT)
			// the behavior of list is similar to queue
			// the first element is the oldest one
			g_random_triangles.pop_front();     // if life_cnt became 0, remove it
	}

	// iterate over the list and make draw calls
	for (auto& curr_geometry : g_random_triangles)
		curr_geometry->DrawObj(curSS);

	// create new triangle and append it to the list
	std::shared_ptr<RandomTriangles> new_triangle;
	new_triangle.reset(new RandomTriangles(3));
	g_random_triangles.push_back(new_triangle);

	glutSwapBuffers();
	checkGlErrors();
}

// Update screen in every millisecond
void timerCallback(int value) {
	glutPostRedisplay();
	glutTimerFunc(1000, timerCallback, 1);
}

void keyboardCallback(unsigned char key, int x, int y) {

	switch (key) {

	case 's':
		// start animation
		std::cout << "Starting animation... \n";
		glutTimerFunc(1000, timerCallback, 1);
		break; 

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