#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

// forward declaration
void initGLUT(int argc, char** argv);
void keyboardCallback(unsigned char key, int x, int y);;
void display();

// global variables
int winID = 0;

typedef struct ShaderState {

} ShaderState;

float vertices[] = {
		0.0f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f
};

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

	// define VBO for vertices of triangle
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// simple Vertex Shader
	const char* simple_vs_source = "#version 130"
		"in vec3 aPos;\n"
		"void main() {\n"
		"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";

	// create and compile vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &simple_vs_source, NULL);
	glCompileShader(vertexShader);

	// simple Fragment Shader
	const char* simple_fs_source = "#version 130"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\0";

	// create and compile fragment shader
	unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &simple_fs_source, NULL);
	glCompileShader(fragShader);

	// link shaders to make 'Shader Program'
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);


	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Although we've already bound buffer but for explanation!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glutMainLoop();   // block here

	return 0;
}

void initGLUT(int argc, char** argv) {	
	glutInit(&argc, argv);

	winID = glutCreateWindow("Test");

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboardCallback);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	glVertex2f(0.0f, 0.5f);
	glVertex2f(-0.5f, -0.5f);
	glVertex2f(0.5f, -0.5f);
	glEnd();
	glFinish();
}

/*
* Keyboard callback
*/
void keyboardCallback(unsigned char key, int x, int y) {

	switch (key) {
	case 'q':
		// quit program
		std::cout << "Recieved 'q'! Terminating... \n";
		glutDestroyWindow(winID);
		exit(EXIT_SUCCESS);
	}
}