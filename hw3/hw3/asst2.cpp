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
#include <cmath>

#include <GL/glew.h>
#ifdef __APPLE__
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

#include "arcball.h"
#include "cvec.h"
#include "matrix4.h"
#include "rigtform.h"
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
static shared_ptr<Geometry> g_ground, g_cube_1, g_cube_2, g_sphere;
static std::vector<shared_ptr<Geometry>> scene;     // (refactor required) later use this to put all scene geometries in one vector

// --------- Scene

static const Cvec3 g_light1(2.0, 3.0, 14.0), g_light2(-2, -3.0, -5.0);  // define two lights positions in world space

// Eye, Object matrices
static RigTForm g_skyRbt = RigTForm(Cvec3(0.0, 0.25, 4.0));
static RigTForm objRbt_1 = RigTForm(Cvec3(0.75, 0, 0.0));
static RigTForm objRbt_2 = RigTForm(Cvec3(-0.75, 0, 0.0));

// World matrix
static RigTForm g_worldRbt = RigTForm(Cvec3(0.0, 0.0, 0.0));
static bool is_worldsky_frame = false;

// list of object matrices
// 1. cube 1
// 2. cube 2
static RigTForm initial_rigs[3] = { g_skyRbt, objRbt_1, objRbt_2 };
static Cvec3f g_objectColors[3] = { Cvec3f(1, 0, 0), Cvec3f(0, 0, 1), Cvec3f(0, 1, 0) };

// list of manipulatable object matrices
static RigTForm manipulatable_obj[3] = { g_skyRbt, objRbt_1, objRbt_2 };

class ViewpointState {
public:
    // Constructor
    ViewpointState() {
        CurrentObjIdx = 1;    // initially cube 1
        CurrentEyeIdx = 0;    // initially cube 2
        updateAuxFrame();     // initial calculation of auxiliary frame
        updateWorldEyeFrame();    // initial calculation of world-eye frame
        IsWorldSkyFrame_ = false;    
    }

    void transformObjWrtA(const RigTForm& M) {
        manipulatable_obj[CurrentObjIdx] = doMtoOwrtA(M, manipulatable_obj[CurrentObjIdx], getAuxFrame());
    }

    /* modify transform according to current auxiliary frame */
    unsigned int getAuxFrameDescriptor() {
        /* Three cases
         * Case 1 - Manipulate cubes
         * Case 2 - Manipulate sky view w.r.t world origin and sky view axes
         * Case 3 - Manipulate sky view w.r.t its origin and axes
         */
        if (isWorldSkyFrame()) {
            // case 2
            return static_cast<unsigned int>(AuxFrameDescriptor::world_sky);
        }
        else if (isSkySkyFrame()) {
            // case 3
            return static_cast<unsigned int>(AuxFrameDescriptor::sky_sky);
        }
        else {
            // case 1
            return static_cast<unsigned int>(AuxFrameDescriptor::cube_other);
        }
    }

    /* setters */
    void switchEye() {
        if (isWorldSkyFrame()) {
            setIsWorldSkyFrame(false);
        }

        CurrentEyeIdx++;
        if (CurrentEyeIdx > 2)
            CurrentEyeIdx = 0;

        if (CurrentEyeIdx != 0 && CurrentObjIdx == 0) {
            // if current eye is a cube and user tries to transform sky camera
            std::cout << "You CANNOT control sky camera with respect to cube! \n";
            CurrentObjIdx = 1;
        }

        if (isSkySkyFrame()) {
            // if current frame is sky-sky frame
            // give a user an option 'm'
            std::cout << "You're now in sky-sky frame\n";
            std::cout << "Press 'm' to switch between world-sky frame and sky-sky frame\n";
        }

        // update auxiliary frame for new viewpoint
        updateAuxFrame();
    }

    void switchObject() {

        if (isWorldSkyFrame()) {
            setIsWorldSkyFrame(false);
        }

        CurrentObjIdx++;
        if (CurrentObjIdx > 2)
            CurrentObjIdx = 0;

        if (CurrentObjIdx != 0 && CurrentObjIdx == 0) {
            // if current eye is a cube and user tries to transform sky camera
            std::cout << "You CANNOT control sky camera with respect to cube! \n";
            CurrentObjIdx = 1;
        }

        if (isSkySkyFrame()) {
            // if current frame is sky-sky frame
            // give a user an option 'm'
            std::cout << "You're now in sky-sky frame\n";
            std::cout << "Press 'm' to switch between world-sky frame and sky-sky frame\n";
        }

        // update auxiliary frame for new object
        updateAuxFrame();
    }

    void setIsWorldSkyFrame(const bool v) {
        // warning you should check input type
        IsWorldSkyFrame_ = v;
    }

    void updateAuxFrame() {
        if (isWorldSkyFrame()) {
            // if current frame is world-eye frame
            updateWorldEyeFrame();
            AuxFrame = WorldEyeFrame;
        }
        else {
            AuxFrame = makeMixedFrame(getCurrentObj(), getCurrentEye());
        }
    }

    void updateWorldEyeFrame() {
        WorldEyeFrame = makeMixedFrame(g_worldRbt, getCurrentEye());
    }

    /* getters */
    RigTForm getCurrentObj() {
        return manipulatable_obj[CurrentObjIdx];
    }

    RigTForm getCurrentEye() {
        return manipulatable_obj[CurrentEyeIdx];
    }

    RigTForm getAuxFrame() {
        return AuxFrame;
    }

    bool isArcballVisible() {
        if (isWorldSkyFrame() || ((CurrentObjIdx == 1 || CurrentObjIdx == 2) && (CurrentObjIdx != CurrentEyeIdx))) {
            // two cases
            // (1) Current auxiliary frame is world-sky frame
            // (2) User is controlling one of the cubes and the current eye is not equal to it
            return true;
        } 
        return false;
    }

    /* utilities */
    bool isSkySkyFrame() {
        return (CurrentEyeIdx == 0) && (CurrentObjIdx == 0);
    }
    
    bool isWorldSkyFrame() {
        return IsWorldSkyFrame_;
    }
    
    void describeCurrentEye() {
        string CurrentEyeName;

        switch (CurrentEyeIdx) {
        case 0:
            CurrentEyeName = "Sky-View";
            break;
        case 1:
            CurrentEyeName = "Cube 1";
            break;
        case 2:
            CurrentEyeName = "Cube 2";
            break;
        }

        std::cout << "Current eye is " << CurrentEyeName << "\n";
        std::cout << "Eye matrix for this camera is: \n";
        printRigTForm(manipulatable_obj[CurrentEyeIdx]);
    }

    void describeCurrentObj() {
        string CurrentObjName;

        switch (CurrentObjIdx) {
        case 0:
            CurrentObjName = "Sky-View";
            break;
        case 1:
            CurrentObjName = "Cube 1";
            break;
        case 2:
            CurrentObjName = "Cube 2";
            break;
        }

        std::cout << "Controlling " << CurrentObjName << "\n";
        std::cout << "Object matrix for this object is: \n";
        printRigTForm(manipulatable_obj[CurrentObjIdx]);
    }

    void describeCurrentAuxFrame() {
        if (isWorldSkyFrame()) {
            std::cout << "Currently in World-Sky frame\n";
        }
        std::cout << "Current auxiliary frame is: \n";
        printRigTForm(getAuxFrame());
    }

    void describeCurrentStatus() {
        std::cout << "================================================\n";
        describeCurrentEye();
        std::cout << "\n";
        describeCurrentObj();
        std::cout << "\n";
        describeCurrentAuxFrame();
        std::cout << "================================================\n";

    }

private:
    bool IsWorldSkyFrame_;
    unsigned int CurrentObjIdx;    // initially cube 1
    unsigned int CurrentEyeIdx;    // initially cube 2

    // RigTForm representation of aux_frame and world_eye_frame
    RigTForm AuxFrame;
    RigTForm WorldEyeFrame;
    enum class AuxFrameDescriptor { cube_other = 1, world_sky, sky_sky };
};

static ViewpointState g_VPState = ViewpointState();

// values related to arcball appearance
static double g_arcballScreenRadius = 0.25 * std::min(g_windowWidth, g_windowHeight);
static double g_arcballScale;

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

static void initSpheres() {
    int slices = 10;
    int stacks = 10;
    int ibLen, vbLen;
    getSphereVbIbLen(slices, stacks, vbLen, ibLen);

    // Temporary storage for sphere geometry
    std::vector<VertexPN> vtx(vbLen);
    std::vector<unsigned short> idx(ibLen);

    // create a sphere for arcball visualization
    makeSphere(1.0, slices, stacks, vtx.begin(), idx.begin());
    g_sphere.reset(new Geometry(&vtx[0], &idx[0], vbLen, ibLen));
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
  const RigTForm eyeRbt = g_VPState.getCurrentEye();
  const RigTForm invEyeRbt = inv(eyeRbt);

  const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(g_light1, 1)); // g_light1 position in eye coordinates
  const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(g_light2, 1)); // g_light2 position in eye coordinates
  safe_glUniform3f(curSS.h_uLight, eyeLight1[0], eyeLight1[1], eyeLight1[2]);
  safe_glUniform3f(curSS.h_uLight2, eyeLight2[0], eyeLight2[1], eyeLight2[2]);

  // draw ground
  // ===========
  //

  // TODO: Find way to replace 'normalMatrix'
  const RigTForm groundRbt = RigTForm();  // identity -> find a way to replace it with RigTForm!
  Matrix4 MVM = RigTFormToMatrix(invEyeRbt * groundRbt);
  Matrix4 NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);
  safe_glUniform3f(curSS.h_uColor, 0.1, 0.95, 0.1); // set color
  g_ground->draw(curSS);

  // draw cubes
  // ==========
  // draw the first one
  MVM = RigTFormToMatrix(invEyeRbt * manipulatable_obj[1]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);

  safe_glUniform3f(curSS.h_uColor, g_objectColors[0][0], g_objectColors[0][1], g_objectColors[0][2]);
  g_cube_1->draw(curSS);

  // draw the second one
  MVM = RigTFormToMatrix(invEyeRbt * manipulatable_obj[2]);
  NMVM = normalMatrix(MVM);
  sendModelViewNormalMatrix(curSS, MVM, NMVM);

  safe_glUniform3f(curSS.h_uColor, g_objectColors[1][0], g_objectColors[1][1], g_objectColors[1][2]);
  g_cube_2->draw(curSS);

  // draw the arcball
  if (g_VPState.isArcballVisible()) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // draw wireframe

      if (g_VPState.isWorldSkyFrame()) {
          // if the user is in world-sky frame, draw arcball at world center
          RigTForm MVRigTForm = invEyeRbt * g_worldRbt;
          MVM = RigTFormToMatrix(MVRigTForm);

          if (!((g_mouseLClickButton && g_mouseRClickButton) || g_mouseMClickButton)) {
              g_arcballScale = getScreenToEyeScale(MVRigTForm.getTranslation()[2],
                  g_frustFovY, g_windowHeight);
          }
      } 

      else {
          // otherwise, sync its position with current object
          RigTForm MVRigTForm = invEyeRbt * g_VPState.getCurrentObj();
          MVM = RigTFormToMatrix(MVRigTForm);

          if (!((g_mouseLClickButton && g_mouseRClickButton) || g_mouseMClickButton)) {
              g_arcballScale = getScreenToEyeScale(MVRigTForm.getTranslation()[2],
                  g_frustFovY, g_windowHeight);
          }
      }
      
      // scale arcball properly
      double scale = g_arcballScale * g_arcballScreenRadius;
      Matrix4 scale_mat = Matrix4::makeScale(Cvec3(scale, scale, scale));


      MVM *= scale_mat;

      NMVM = normalMatrix(MVM);
      sendModelViewNormalMatrix(curSS, MVM, NMVM);

      safe_glUniform3f(curSS.h_uColor, g_objectColors[2][0], g_objectColors[2][1], g_objectColors[2][2]);
      g_sphere->draw(curSS);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // end wireframe mode
  }
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
  // update the size of the arcball
  g_arcballScreenRadius = 0.25 * std::min(g_windowWidth, g_windowHeight);
  cerr << "Size of window is now " << w << "x" << h << endl;
  updateFrustFovY();
  glutPostRedisplay();
}

/* Forward declaration for motion helpers */
static RigTForm ArcballInterfaceRotation(const int x, const int y);
static RigTForm ArcballInterfaceTranslation(const int x, const int y);
static RigTForm DefaultInterfaceRotation(const int x, const int y);
static RigTForm DefaultInterfaceTranslation(const int x, const int y);

static void motion(const int x, const int y) {
    RigTForm m;

    if (g_VPState.isArcballVisible()) {
        // rotation when arcball is visible

        if (g_mouseLClickButton && !g_mouseRClickButton) {
            m = ArcballInterfaceRotation(x, y);
        }

        // translation when arcball is visible
        else {
            m = ArcballInterfaceTranslation(x, y);
        }
    }
    
    else {
        // interface when arcball is invisible

        if (g_mouseLClickButton && !g_mouseRClickButton) {
            // left button down. rotation
            m = DefaultInterfaceRotation(x, y);
        }
        else {
            // right button down. translation on xy plane or along z axis
            m = DefaultInterfaceTranslation(x, y);
        }
    }

  if (g_mouseClickDown) {
      g_VPState.transformObjWrtA(m);
      g_VPState.updateAuxFrame();

      glutPostRedisplay(); // we always redraw if we changed the scene
  }

  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;
}

/* Helper functions for motions */

static RigTForm ArcballInterfaceRotation(const int x, const int y) {
    // rotation when arcball is visible
        Quat rotation = Quat();

        RigTForm eyeRbt = g_VPState.getCurrentEye();
        RigTForm invEyeRbt = inv(eyeRbt);
        Cvec3 center_eye_coord = Cvec3();

        if (!g_VPState.isWorldSkyFrame()) {
            // in cube-eye frame
            center_eye_coord = (invEyeRbt * g_VPState.getCurrentObj()).getTranslation();
        }
        else {
            // in world-eye frames
            center_eye_coord = (invEyeRbt * g_worldRbt).getTranslation();
        }

        Cvec2 center_screen_coord = getScreenSpaceCoord(center_eye_coord, makeProjectionMatrix(),
            g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight);

        // calculate z coordinate of clicked points in screen coordinate

        int v1_x = (int)(g_mouseClickX - center_screen_coord(0));
        int v1_y = (int)(g_mouseClickY - center_screen_coord(1));
        int v1_z = calculateScreenZ(g_arcballScreenRadius, g_mouseClickX, g_mouseClickY, center_screen_coord);

        // !!!!! Caution: Flip y before using it !!!!!
        int v2_x = (int)(x - center_screen_coord(0));
        int v2_y = (int)(g_windowHeight - y - 1 - center_screen_coord(1));
        int v2_z = calculateScreenZ(g_arcballScreenRadius, x, g_windowHeight - y - 1, center_screen_coord);

        Cvec3 v1;
        Cvec3 v2;
        Cvec3 k;

        if (v1_z < 0 || v2_z < 0) {
            // user points outside the arcball
            v1 = normalize(Cvec3(v1_x, v1_y, 0));
            v2 = normalize(Cvec3(v2_x, v2_y, 0));
            k = cross(v1, v2);

            rotation = Quat(dot(v1, v2), k);
        }

        else {
            v1 = normalize(Cvec3(v1_x, v1_y, v1_z));
            v2 = normalize(Cvec3(v2_x, v2_y, v2_z));
            k = cross(v1, v2);

            rotation = Quat(dot(v1, v2), k);

            if (g_VPState.isWorldSkyFrame()) {
                rotation = inv(rotation);
            }
        }

        return RigTForm(rotation);
}

static RigTForm ArcballInterfaceTranslation(const int x, const int y) {

    RigTForm m;
    const double dx = x - g_mouseClickX;
    const double dy = g_windowHeight - y - 1 - g_mouseClickY;

    if (g_mouseRClickButton && !g_mouseLClickButton) { // right button down?
        switch (g_VPState.getAuxFrameDescriptor()) {
        case 1:
            // default behavior
            m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * g_arcballScale);
            break;
        case 2:
            // invert sign of rotation and translation
            m = RigTForm::makeTranslation(-Cvec3(dx, dy, 0) * g_arcballScale);
            break;

        case 3:
            // invert sign of rotation only
            m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * g_arcballScale);
            break;
        }
    }
    else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {  // middle or (left and right) button down?
        switch (g_VPState.getAuxFrameDescriptor()) {
        case 1:
            // default behavior
            m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * g_arcballScale);
            break;
        case 2:
            // invert sign of rotation and translation
            m = RigTForm::makeTranslation(-Cvec3(0, 0, -dy) * g_arcballScale);
            break;
        case 3:
            // invert sign of rotation only
            m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * g_arcballScale);
            break;
        }
    }

    return m;
}

static RigTForm DefaultInterfaceRotation(const int x, const int y) {
    
    RigTForm m;

    const double dx = x - g_mouseClickX;
    const double dy = g_windowHeight - y - 1 - g_mouseClickY;

    switch (g_VPState.getAuxFrameDescriptor()) {
    case 1:
        // default behavior
        m = RigTForm::makeXRotation(-dy) * RigTForm::makeYRotation(dx);
        break;

    case 2:
        // invert sign of rotation and translation
        m = RigTForm::makeXRotation(dy) * RigTForm::makeYRotation(-dx);
        break;

    case 3:
        // invert sign of rotation only
        m = RigTForm::makeXRotation(dy) * RigTForm::makeYRotation(-dx);
        break;
    }

    return m;
}

static RigTForm DefaultInterfaceTranslation(const int x, const int y) {
    
    RigTForm m;

    const double dx = x - g_mouseClickX;
    const double dy = g_windowHeight - y - 1 - g_mouseClickY;

    if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {
        // middle or both left-right button down?
        switch (g_VPState.getAuxFrameDescriptor()) {
        case 1:
            // default behavior
            m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * 0.01);
            break;
        case 2:
            // invert sign of rotation and translation
            m = RigTForm::makeTranslation(-Cvec3(0, 0, -dy) * 0.01);
            break;
        case 3:
            // invert sign of rotation only
            m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * 0.01);
            break;
        }
    }

    else {
        // right button down?
        switch (g_VPState.getAuxFrameDescriptor()) {
        case 1:
            // default behavior
            m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * 0.01);
            break;
        case 2:
            // invert sign of rotation and translation
            m = RigTForm::makeTranslation(-Cvec3(dx, dy, 0) * 0.01);
            break;

        case 3:
            // invert sign of rotation only
            m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * 0.01);
            break;
        }
    }
    
    return m;
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

  glutPostRedisplay();
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
        g_VPState.switchEye();

        // describe current state
        g_VPState.describeCurrentStatus();
        break;

    case 'o':
        std::cout << "Pressed 'o'! Switching object\n";

        // switch object
        g_VPState.switchObject();

        // describe current state
        g_VPState.describeCurrentStatus();
        break;

    case 'm':
        if (!g_VPState.isSkySkyFrame()) {
            // current frame is not a sky-sky frame
            std::cout << "You can use this option ONLY when you're in sky-sky frame\n";
        }
        else {
            // current frame is a sky-sky frame
            if (!g_VPState.isWorldSkyFrame()) {
                // current frame is a sky-sky frame -> switching to world-sky frame
                std::cout << "Switching to World-Sky frame\n";
                g_VPState.setIsWorldSkyFrame(true);
                g_VPState.updateAuxFrame();
                g_VPState.describeCurrentStatus();
            }
            else {
                // current frame is a world-sky frame -> switching to sky-sky frame
                std::cout << "Switching to Sky-Sky frame\n";
                g_VPState.setIsWorldSkyFrame(false);
                g_VPState.updateAuxFrame();
                g_VPState.describeCurrentStatus();
            }
        }

        break;

    case 'r':
        // reset object position
        std::cout << "Pressed 'r'! Resetting all object & eye position\n";
        
        for (int i = 0; i < 3; ++i) {
            manipulatable_obj[i] = initial_rigs[i];
        }
        g_VPState.updateAuxFrame();

        g_VPState.describeCurrentStatus();
        break;

    case 'd':
        g_VPState.describeCurrentStatus();
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
    glutCreateWindow("Assignment 3");                       // title the window

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
    initSpheres();
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