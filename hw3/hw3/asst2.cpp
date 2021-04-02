////////////////////////////////////////////////////////////////////////
//
//   Harvard University
//   CS175 : Computer Graphics
//   Professor Steven Gortler
//
////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

#include <GL/glew.h>
#ifdef __APPLE__
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "cvec.h"
#include "matrix4.h"
#include "geometrymaker.h"
#include "rigtform.h"
#include "ppm.h"
#include "glsupport.h"

using namespace std;      // for string, vector, iostream, and other standard C++ stuff

// G L O B A L S ///////////////////////////////////////////////////

// --------- IMPORTANT --------------------------------------------------------
// Before you start working on this assignment, set the following variable
// properly to indicate whether you want to use OpenGL 2.x with GLSL 1.0 or
// OpenGL 3.x+ with GLSL 1.3.
//
// Set g_Gl2Compatible = true to use GLSL 1.0 and g_Gl2Compatible = false to
// use GLSL 1.3. Make sure that your machine supports the version of GLSL you
// are using. In particular, on Mac OS X currently there is no way of using
// OpenGL 3.x with GLSL 1.3 when GLUT is used.
//
// If g_Gl2Compatible=true, shaders with -gl2 suffix will be loaded.
// If g_Gl2Compatible=false, shaders with -gl3 suffix will be loaded.
// To complete the assignment you only need to edit the shader files that get
// loaded
// ----------------------------------------------------------------------------
static const bool g_Gl2Compatible = false;


static const float g_frustMinFov = 60.0;  // A minimal of 60 degree field of view
static float g_frustFovY = g_frustMinFov; // FOV in y direction (updated by updateFrustFovY)

static const float g_frustNear = -0.1;    // near plane
static const float g_frustFar = -50.0;    // far plane
static const float g_groundY = -2.0;      // y coordinate of the ground
static const float g_groundSize = 10.0;   // half the ground length

static int g_windowWidth = 512;
static int g_windowHeight = 512;
static bool g_mouseClickDown = false;    // is the mouse button pressed
static bool g_mouseLClickButton, g_mouseRClickButton, g_mouseMClickButton;
static int g_mouseClickX, g_mouseClickY; // coordinates for mouse click event
static int g_activeShader = 0;

struct ShaderState {
  GlProgram program;

  // Handles to uniform variables
  GLint h_uLight, h_uLight2;
  GLint h_uProjMatrix;
  GLint h_uModelViewMatrix;
  GLint h_uNormalMatrix;
  GLint h_uColor;

  // Handles to vertex attributes
  GLint h_aPosition;
  GLint h_aNormal;

  ShaderState(const char* vsfn, const char* fsfn) {
    readAndCompileShader(program, vsfn, fsfn);

    const GLuint h = program; // short hand

    // Retrieve handles to uniform variables
    h_uLight = safe_glGetUniformLocation(h, "uLight");
    h_uLight2 = safe_glGetUniformLocation(h, "uLight2");
    h_uProjMatrix = safe_glGetUniformLocation(h, "uProjMatrix");
    h_uModelViewMatrix = safe_glGetUniformLocation(h, "uModelViewMatrix");
    h_uNormalMatrix = safe_glGetUniformLocation(h, "uNormalMatrix");
    h_uColor = safe_glGetUniformLocation(h, "uColor");

    // Retrieve handles to vertex attributes
    h_aPosition = safe_glGetAttribLocation(h, "aPosition");
    h_aNormal = safe_glGetAttribLocation(h, "aNormal");

    if (!g_Gl2Compatible)
      glBindFragDataLocation(h, 0, "fragColor");
    checkGlErrors();
  }

};

static const int g_numShaders = 2;
static const char * const g_shaderFiles[g_numShaders][2] = {
  {"./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader"},
  {"./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader"}
};
static const char * const g_shaderFilesGl2[g_numShaders][2] = {
  {"./shaders/basic-gl2.vshader", "./shaders/diffuse-gl2.fshader"},
  {"./shaders/basic-gl2.vshader", "./shaders/solid-gl2.fshader"}
};
static vector<shared_ptr<ShaderState> > g_shaderStates; // our global shader states

// --------- Geometry

// Macro used to obtain relative offset of a field within a struct
#define FIELD_OFFSET(StructType, field) &(((StructType *)0)->field)

// A vertex with floating point position and normal
struct VertexPN {
  Cvec3f p, n;

  VertexPN() {}
  VertexPN(float x, float y, float z,
           float nx, float ny, float nz)
    : p(x,y,z), n(nx, ny, nz)
  {}

  // Define copy constructor and assignment operator from GenericVertex so we can
  // use make* functions from geometrymaker.h
  VertexPN(const GenericVertex& v) {
    *this = v;
  }

  VertexPN& operator = (const GenericVertex& v) {
    p = v.pos;
    n = v.normal;
    return *this;
  }
};

struct Geometry {
  GlBufferObject vbo, ibo;
  int vboLen, iboLen;

  Geometry(VertexPN *vtx, unsigned short *idx, int vboLen, int iboLen) {
    this->vboLen = vboLen;
    this->iboLen = iboLen;

    // Now create the VBO and IBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vboLen, vtx, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * iboLen, idx, GL_STATIC_DRAW);
  }

  void draw(const ShaderState& curSS) {
    // Enable the attributes used by our shader
    safe_glEnableVertexAttribArray(curSS.h_aPosition);
    safe_glEnableVertexAttribArray(curSS.h_aNormal);

    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    safe_glVertexAttribPointer(curSS.h_aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, p));
    safe_glVertexAttribPointer(curSS.h_aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), FIELD_OFFSET(VertexPN, n));

    // bind ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // draw!
    glDrawElements(GL_TRIANGLES, iboLen, GL_UNSIGNED_SHORT, 0);

    // Disable the attributes used by our shader
    safe_glDisableVertexAttribArray(curSS.h_aPosition);
    safe_glDisableVertexAttribArray(curSS.h_aNormal);
  }
};

// Vertex buffer and index buffer associated with the ground and cube geometry
static shared_ptr<Geometry> g_ground, g_cube_1, g_cube_2;
static std::vector<shared_ptr<Geometry>> scene;     // (refactor required) later use this to put all scene geometries in one vector

// --------- Scene

static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);  // define two lights positions in world space

// Eye, Object matrices
static RigTForm g_skyRbt = RigTForm(Cvec3(0.0, 0.25, 4.0));
static RigTForm objRbt_1 = RigTForm(Cvec3(0.75, 0, 0));
static RigTForm objRbt_2 = RigTForm(Cvec3(-0.75, 0, 0));

// World matrix
static RigTForm g_worldRbt = RigTForm(Cvec3(0.0, 0.0, 0.0));
static bool is_worldsky_frame = false;

// list of object matrices
// 1. cube 1
// 2. cube 2
static RigTForm initial_rigs[3] = { g_skyRbt, objRbt_1, objRbt_2 };
static Cvec3f g_objectColors[2] = { Cvec3f(1, 0, 0), Cvec3f(0, 0, 1) };

// list of manipulatable object matrices
static RigTForm manipulatable_obj[3] = { g_skyRbt, objRbt_1, objRbt_2 };

class ViewpointState {
public:
    // Constructor
    ViewpointState() {
        current_obj_idx = 1;    // initially cube 1
        current_eye_idx = 0;    // initially cube 2
        update_aux_frame();     // initial calculation of auxiliary frame
        update_world_eye_frame();    // initial calculation of world-eye frame
        is_world_sky_frame_ = false;    
    }

    void transform_obj_wrt_A(const RigTForm& M) {
        manipulatable_obj[current_obj_idx] = doMtoOwrtA(M, manipulatable_obj[current_obj_idx], get_aux_frame());
    }

    /* modify transform according to current auxiliary frame */
    unsigned int get_aux_frame_descriptor() {
        /* Three cases
         * Case 1 - Manipulate cubes
         * Case 2 - Manipulate sky view w.r.t world origin and sky view axes
         * Case 3 - Manipulate sky view w.r.t its origin and axes
         */
        if (is_world_sky_frame()) {
            // case 2
            return static_cast<unsigned int>(aux_frame_descriptor::world_sky);
        }
        else if (is_sky_sky_frame()) {
            // case 3
            return static_cast<unsigned int>(aux_frame_descriptor::sky_sky);
        }
        else {
            // case 1
            return static_cast<unsigned int>(aux_frame_descriptor::cube_other);
        }
    }

    /* setters */
    void switch_eye() {
        if (is_world_sky_frame()) {
            set_is_world_sky_frame(false);
        }

        current_eye_idx++;
        if (current_eye_idx > 2)
            current_eye_idx = 0;

        if (current_eye_idx != 0 && current_obj_idx == 0) {
            // if current eye is a cube and user tries to transform sky camera
            std::cout << "You CANNOT control sky camera with respect to cube! \n";
            current_obj_idx = 1;
        }

        if (is_sky_sky_frame()) {
            // if current frame is sky-sky frame
            // give a user an option 'm'
            std::cout << "You're now in sky-sky frame\n";
            std::cout << "Press 'm' to switch between world-sky frame and sky-sky frame\n";
        }

        // update auxiliary frame for new viewpoint
        update_aux_frame();
    }

    void switch_obj() {

        if (is_world_sky_frame()) {
            set_is_world_sky_frame(false);
        }

        current_obj_idx++;
        if (current_obj_idx > 2)
            current_obj_idx = 0;

        if (current_eye_idx != 0 && current_obj_idx == 0) {
            // if current eye is a cube and user tries to transform sky camera
            std::cout << "You CANNOT control sky camera with respect to cube! \n";
            current_obj_idx = 1;
        }

        if (is_sky_sky_frame()) {
            // if current frame is sky-sky frame
            // give a user an option 'm'
            std::cout << "You're now in sky-sky frame\n";
            std::cout << "Press 'm' to switch between world-sky frame and sky-sky frame\n";
        }

        // update auxiliary frame for new object
        update_aux_frame();
    }

    void set_is_world_sky_frame(const bool v) {
        // warning you should check input type
        is_world_sky_frame_ = v;
    }

    void update_aux_frame() {
        if (is_world_sky_frame()) {
            // if current frame is world-eye frame
            update_world_eye_frame();
            aux_frame = world_eye_frame;
        }
        else {
            aux_frame = makeMixedFrame(get_current_obj(), get_current_eye());
        }
    }

    void update_world_eye_frame() {
        world_eye_frame = makeMixedFrame(g_worldRbt, get_current_eye());
    }

    /* getters */
    RigTForm get_current_obj() {
        return manipulatable_obj[current_obj_idx];
    }

    RigTForm get_current_eye() {
        return manipulatable_obj[current_eye_idx];
    }

    RigTForm get_aux_frame() {
        return aux_frame;
    }

    /* utilities */
    bool is_sky_sky_frame() {
        return current_eye_idx == 0 && current_obj_idx == 0;
    }
    
    bool is_world_sky_frame() {
        return is_world_sky_frame_;
    }
    
    void describe_current_eye() {
        string current_eye_name;

        switch (current_eye_idx) {
        case 0:
            current_eye_name = "Sky-View";
            break;
        case 1:
            current_eye_name = "Cube 1";
            break;
        case 2:
            current_eye_name = "Cube 2";
            break;
        }

        std::cout << "Current eye is " << current_eye_name << "\n";
        std::cout << "Eye matrix for this camera is: \n";
        // printMatrix4(manipulatable_obj[current_eye_idx]);
    }

    void describe_current_obj() {
        string current_obj_name;

        switch (current_obj_idx) {
        case 0:
            current_obj_name = "Sky-View";
            break;
        case 1:
            current_obj_name = "Cube 1";
            break;
        case 2:
            current_obj_name = "Cube 2";
            break;
        }

        std::cout << "Controlling " << current_obj_name << "\n";
        std::cout << "Object matrix for this object is: \n";
        // printMatrix4(manipulatable_obj[current_obj_idx]);
    }

    void describe_current_aux() {
        if (is_world_sky_frame()) {
            std::cout << "Currently in World-Sky frame\n";
        }
        std::cout << "Current auxiliary frame is: \n";
        // printMatrix4(get_aux_frame());
    }

    void describe_current_status() {
        std::cout << "================================================\n";
        describe_current_eye();
        describe_current_obj();
        describe_current_aux();
        std::cout << "================================================\n";

    }

private:
    bool is_world_sky_frame_;
    unsigned int current_obj_idx;    // initially cube 1
    unsigned int current_eye_idx;    // initially cube 2

    // RigTForm representation of aux_frame and world_eye_frame
    RigTForm aux_frame;
    RigTForm world_eye_frame;
    enum class aux_frame_descriptor { cube_other = 1, world_sky, sky_sky };
};

// Refactoring --> All view-obj information will be incapsulated in here!
static ViewpointState g_VPState = ViewpointState();

///////////////// END OF G L O B A L S //////////////////////////////////////////////////

static void initGround() {
  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
  VertexPN vtx[4] = {
    VertexPN(-g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
    VertexPN(-g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
    VertexPN( g_groundSize, g_groundY,  g_groundSize, 0, 1, 0),
    VertexPN( g_groundSize, g_groundY, -g_groundSize, 0, 1, 0),
  };
  unsigned short idx[] = {0, 1, 2, 0, 2, 3};
  g_ground.reset(new Geometry(&vtx[0], &idx[0], 4, 6));
}

static void initCubes() {
  int ibLen, vbLen;
  getCubeVbIbLen(vbLen, ibLen);

  // Temporary storage for cube geometry
  vector<VertexPN> vtx(vbLen);
  vector<unsigned short> idx(ibLen);

  // create the first cube
  makeCube(1, vtx.begin(), idx.begin());
  g_cube_1.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));

  // create the second cube
  makeCube(1, vtx.begin(), idx.begin());
  g_cube_2.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
}

// takes a projection matrix and send to the the shaders
static void sendProjectionMatrix(const ShaderState& curSS, const Matrix4& projMatrix) {
  GLfloat glmatrix[16];
  projMatrix.writeToColumnMajorMatrix(glmatrix); // send projection matrix
  safe_glUniformMatrix4fv(curSS.h_uProjMatrix, glmatrix);
}

// takes MVM and its normal matrix to the shaders
static void sendModelViewNormalMatrix(const ShaderState& curSS, const Matrix4& MVM, const Matrix4& NMVM) {
  GLfloat glmatrix[16];
  MVM.writeToColumnMajorMatrix(glmatrix); // send MVM
  safe_glUniformMatrix4fv(curSS.h_uModelViewMatrix, glmatrix);

  NMVM.writeToColumnMajorMatrix(glmatrix); // send NMVM
  safe_glUniformMatrix4fv(curSS.h_uNormalMatrix, glmatrix);
}

// update g_frustFovY from g_frustMinFov, g_windowWidth, and g_windowHeight
static void updateFrustFovY() {
  if (g_windowWidth >= g_windowHeight)
    g_frustFovY = g_frustMinFov;
  else {
    const double RAD_PER_DEG = 0.5 * CS175_PI/180;
    g_frustFovY = atan2(sin(g_frustMinFov * RAD_PER_DEG) * g_windowHeight / g_windowWidth, cos(g_frustMinFov * RAD_PER_DEG)) / RAD_PER_DEG;
  }
}

static Matrix4 makeProjectionMatrix() {
  return Matrix4::makeProjection(
           g_frustFovY, g_windowWidth / static_cast <double> (g_windowHeight),
           g_frustNear, g_frustFar);
}

static void drawStuff() {
  // short hand for current shader state
  const ShaderState& curSS = *g_shaderStates[g_activeShader];

  // build & send proj. matrix to vshader
  const Matrix4 projmat = makeProjectionMatrix();
  sendProjectionMatrix(curSS, projmat);

  // use the skyRbt as the eyeRbt
  const Matrix4 eyeRbt = g_VPState.get_current_eye_matrix();
  const Matrix4 invEyeRbt = inv(eyeRbt);

  const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1)); // g_light1 position in eye coordinates
  const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1)); // g_light2 position in eye coordinates
  safe_glUniform3f(curSS.h_uLight, eyeLight1[0], eyeLight1[1], eyeLight1[2]);
  safe_glUniform3f(curSS.h_uLight2, eyeLight2[0], eyeLight2[1], eyeLight2[2]);

  // draw ground
  // ===========
  //
  const Matrix4 groundRbt = Matrix4();  // identity
  Matrix4 MVM = invEyeRbt * groundRbt;
  Matrix4 NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, 0.1, 0.95, 0.1); // set color
  g_ground->draw(curSS);

  // draw cubes
  // ==========
  // draw the first one
  MVM = invEyeRbt * manipulatable_obj[1];
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);

  safe_glUniform3f(curSS.h_uColor, g_objectColors[0][0], g_objectColors[0][1], g_objectColors[0][2]);
  g_cube_1->draw(curSS);

  // draw the second one
  MVM = invEyeRbt * manipulatable_obj[2];
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);

  safe_glUniform3f(curSS.h_uColor, g_objectColors[1][0], g_objectColors[1][1], g_objectColors[1][2]);
  g_cube_2->draw(curSS);
}

/* GLUT callbacks */

static void display() {
  glUseProgram(g_shaderStates[g_activeShader]->program);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // clear framebuffer color&depth

  drawStuff();

  glutSwapBuffers();                                    // show the back buffer (where we rendered stuff)

  checkGlErrors();
}

static void reshape(const int w, const int h) {
  g_windowWidth = w;
  g_windowHeight = h;
  glViewport(0, 0, w, h);
  cerr << "Size of window is now " << w << "x" << h << endl;
  updateFrustFovY();
  glutPostRedisplay();
}

static void motion(const int x, const int y) {
  const double dx = x - g_mouseClickX;
  const double dy = g_windowHeight - y - 1 - g_mouseClickY;

  Matrix4 m;
  if (g_mouseLClickButton && !g_mouseRClickButton) { // left button down?
      switch (g_VPState.get_aux_frame_descriptor()) {
      case 1:
          // default behavior
          m = Matrix4::makeXRotation(-dy) * Matrix4::makeYRotation(dx);
          break;

      case 2:
          // invert sign of rotation and translation
          m = Matrix4::makeXRotation(dy) * Matrix4::makeYRotation(-dx);
          break;

      case 3:
          // invert sign of rotation only
          m = Matrix4::makeXRotation(dy) * Matrix4::makeYRotation(-dx);
          break;
      }
  }
  else if (g_mouseRClickButton && !g_mouseLClickButton) { // right button down?
      switch (g_VPState.get_aux_frame_descriptor()) {
      case 1:
          // default behavior
          m = Matrix4::makeTranslation(Cvec3(dx, dy, 0) * 0.01);
          break;
      case 2:
          // invert sign of rotation and translation
          m = Matrix4::makeTranslation(-Cvec3(dx, dy, 0) * 0.01);
          break;

      case 3:
          // invert sign of rotation only
          m = Matrix4::makeTranslation(Cvec3(dx, dy, 0) * 0.01);
          break;
      }
  }
  else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {  // middle or (left and right) button down?
      switch (g_VPState.get_aux_frame_descriptor()) {
      case 1:
          // default behavior
          m = Matrix4::makeTranslation(Cvec3(0, 0, -dy) * 0.01);
          break;
      case 2:
          // invert sign of rotation and translation
          m = Matrix4::makeTranslation(-Cvec3(0, 0, -dy) * 0.01);
          break;
      case 3:
          // invert sign of rotation only
          m = Matrix4::makeTranslation(Cvec3(0, 0, -dy) * 0.01);
          break;
      }
  }

  if (g_mouseClickDown) {

      g_VPState.transform_obj_wrt_A(m);
      g_VPState.update_aux_frame();

      glutPostRedisplay(); // we always redraw if we changed the scene
  }

  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;
}

static void mouse(const int button, const int state, const int x, const int y) {
  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;  // conversion from GLUT window-coordinate-system to OpenGL window-coordinate-system

  g_mouseLClickButton |= (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN);
  g_mouseRClickButton |= (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN);
  g_mouseMClickButton |= (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN);

  g_mouseLClickButton &= !(button == GLUT_LEFT_BUTTON && state == GLUT_UP);
  g_mouseRClickButton &= !(button == GLUT_RIGHT_BUTTON && state == GLUT_UP);
  g_mouseMClickButton &= !(button == GLUT_MIDDLE_BUTTON && state == GLUT_UP);

  g_mouseClickDown = g_mouseLClickButton || g_mouseRClickButton || g_mouseMClickButton;
}

static void keyboard(const unsigned char key, const int x, const int y) {

    switch (key) {

    case 27:
        // quit application
        exit(0);                                  // ESC

    case 'q':
        // quit application
        exit(0);                                  // Quit on 'q'

    case 'h':
        // print out help
        cout << " ============== H E L P ==============\n\n"
            << "h\t\thelp menu\n"
            << "s\t\tsave screenshot\n"
            << "f\t\tToggle flat shading on/off.\n"
            << "o\t\tCycle object to edit\n"
            << "v\t\tCycle view\n"
            << "d\t\tDescribe current eye, object matrices\n"
            << "r\t\tReset the position of current object\n"
            << "drag left mouse to rotate\n" << endl;
        break;

    case 's':
        // capture screen
        glFlush();
        writePpmScreenshot(g_windowWidth, g_windowHeight, "out.ppm");
        break;

    case 'f':
        // enable/disenable shader
        g_activeShader ^= 1;
        break;

    case 'v':
        std::cout << "Pressed 'v'! Switching camera\n";
        
        // switch view point
        g_VPState.switch_eye();

        // describe current state
        g_VPState.describe_current_status();
        break;

    case 'o':
        std::cout << "Pressed 'o'! Switching object\n";

        // switch object
        g_VPState.switch_obj();

        // describe current state
        g_VPState.describe_current_status();
        break;

    case 'm':
        if (!g_VPState.is_sky_sky_frame()) {
            // current frame is not a sky-sky frame
            std::cout << "You can use this option ONLY when you're in sky-sky frame\n";
        }
        else {
            // current frame is a sky-sky frame
            if (!g_VPState.is_world_sky_frame()) {
                // current frame is a sky-sky frame -> switching to world-sky frame
                std::cout << "Switching to World-Sky frame\n";
                g_VPState.set_is_world_sky_frame(true);
                g_VPState.update_aux_frame();
                g_VPState.describe_current_status();
            }
            else {
                // current frame is a world-sky frame -> switching to sky-sky frame
                std::cout << "Switching to Sky-Sky frame\n";
                g_VPState.set_is_world_sky_frame(false);
                g_VPState.update_aux_frame();
                g_VPState.describe_current_status();
            }
        }

        break;

    case 'r':
        // reset object position
        std::cout << "Pressed 'r'! Resetting all object & eye position\n";
        
        for (int i = 0; i < 3; ++i) {
            manipulatable_obj[i] = initial_rigs[i];
        }
        g_VPState.update_aux_frame();

        g_VPState.describe_current_status();
        break;

    case 'd':
        g_VPState.describe_current_status();
        break;
    }
    glutPostRedisplay();
}

/* End of GLUT callbacks */

/* Main program routines */

static void initGlutState(int argc, char* argv[]) {
    glutInit(&argc, argv);                                  // initialize Glut based on cmd-line args
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  //  RGBA pixel channels and double buffering
    glutInitWindowSize(g_windowWidth, g_windowHeight);      // create a window
    glutCreateWindow("Assignment 2");                       // title the window

    glutDisplayFunc(display);                               // display rendering callback
    glutReshapeFunc(reshape);                               // window reshape callback
    glutMotionFunc(motion);                                 // mouse movement callback
    glutMouseFunc(mouse);                                   // mouse click callback
    glutKeyboardFunc(keyboard);
}

static void initGLState() {
    glClearColor(128. / 255., 200. / 255., 255. / 255., 0.);
    glClearDepth(0.);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glReadBuffer(GL_BACK);
    if (!g_Gl2Compatible)
        glEnable(GL_FRAMEBUFFER_SRGB);
}

static void initShaders() {
    g_shaderStates.resize(g_numShaders);
    for (int i = 0; i < g_numShaders; ++i) {
        if (g_Gl2Compatible)
            g_shaderStates[i].reset(new ShaderState(g_shaderFilesGl2[i][0], g_shaderFilesGl2[i][1]));
        else
            g_shaderStates[i].reset(new ShaderState(g_shaderFiles[i][0], g_shaderFiles[i][1]));
    }
}

static void initGeometry() {
    initGround();
    initCubes();
}

int main(int argc, char* argv[]) {
    try {
        initGlutState(argc, argv);

        glewInit(); // load the OpenGL extensions

        cout << (g_Gl2Compatible ? "Will use OpenGL 2.x / GLSL 1.0" : "Will use OpenGL 3.x / GLSL 1.3") << endl;
        if ((!g_Gl2Compatible) && !GLEW_VERSION_3_0)
            throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.3");
        else if (g_Gl2Compatible && !GLEW_VERSION_2_0)
            throw runtime_error("Error: card/driver does not support OpenGL Shading Language v1.0");

        initGLState();
        initShaders();
        initGeometry();

        glutMainLoop();
        return 0;
    }
    catch (const runtime_error& e) {
        cout << "Exception caught: " << e.what() << endl;
        return -1;
    }
}

/* End of main program routine */