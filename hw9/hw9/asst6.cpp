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
#include <cstdlib>
#include <cmath>
#include <random>

#include <GL/glew.h>
#ifdef __APPLE__
#   include <GLUT/glut.h>
#else
#   include <GL/glut.h>
#endif

// assignment 1
#include "ppm.h"
#include "glsupport.h"

// assignment 2
#include "cvec.h"
#include "matrix4.h"

// assignment 3
#include "geometrymaker.h"
#include "rigtform.h"
#include "arcball.h"

// assignment 4
#include "asstcommon.h"
#include "scenegraph.h"
#include "drawer.h"
#include "picker.h"

// assignment 5
#include "animation.h"

// assignment 6
#include "geometry.h"    // revised
#include "material.h"
#include "uniforms.h"

// assignment 7
#include "mesh.h"

#define PI 3.141592

using namespace std;

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

const bool g_Gl2Compatible = true;

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

// Materials
static std::shared_ptr<Material> g_redDiffuseMat, g_blueDiffuseMat, g_bumpFloorMat,
                                g_arcballMat, g_pickingMat, g_lightMat, g_purpleSpecularMat;
std::shared_ptr<Material> g_overridingMaterial;    // used for uniform material'ing' in picking mode
static std::shared_ptr<Material> g_bunnyMat;
static std::vector<std::shared_ptr<Material>> g_bunnyShellMats;

// Geometry
typedef SgGeometryShapeNode MyShapeNode;

// Vertex buffer and index buffer associated with the ground and cube geometry
static shared_ptr<Geometry> g_ground, g_cube, g_sphere;
static std::shared_ptr<SimpleGeometryPN> g_bunnyGeometry;
static std::vector<std::shared_ptr<SimpleGeometryPNX> > g_bunnyShellGeometries;

// Bunny geometry parameters
static const int g_numShells = 24; // constants defining how many layers of shells
static double g_furHeight = 0.21;
static double g_hairyness = 0.7;

// Mesh object for holding bunny mesh
static Mesh g_bunnyMesh;

// Scene graph nodes
static std::shared_ptr<SgRootNode> g_world;
static std::shared_ptr<SgRbtNode> g_skyNode, g_groundNode, g_robot1Node, g_robot2Node;
static std::shared_ptr<SgRbtNode> g_light1Node, g_light2Node;
static std::shared_ptr<SgRbtNode> g_bunnyNode;
static std::shared_ptr<SgRbtNode> g_currentEyeNode;
static std::shared_ptr<SgRbtNode> g_currentPickedRbtNode;

static std::vector<std::shared_ptr<SgRbtNode>> Eyes = { g_skyNode, g_robot1Node, g_robot2Node };

// Arcball
static Cvec3 g_arcballColor = Cvec3(0, 1, 0);
static float g_arcballScreenRadius = 0.25 * std::min(g_windowWidth, g_windowHeight);
static float g_arcballScale = 0.01;

// Toggle picking
static bool g_isPicking = false;

// Toggle World-Sky frame
static bool g_isWorldSky = false;

// --------- Animation
static Animation::KeyframeList g_keyframes = Animation::KeyframeList();
static std::vector<std::shared_ptr<SgRbtNode>> g_sceneRbtVector = std::vector<std::shared_ptr<SgRbtNode>>();

static int g_msBetweenKeyFrames = 2000;    // 2 seconds between keyframes
static float g_deformSpeed = 500;
static int g_animationFramesPerSecond = 60;    // frames to render per second during animation
static bool g_playing = false;

// Assignment 9
// Global variables for used physical simulation
static const Cvec3 g_gravity(0, -0.5, 0);  // gavity vector
static double g_timeStep = 0.02;
static double g_numStepsPerFrame = 10;
static double g_damping = 0.96;
static double g_stiffness = 4;
static int g_simulationsPerSecond = 60;

static std::vector<Cvec3> g_tipPos,        // should be hair tip pos in world-space coordinates
g_tipVelocity;   // should be hair tip velocity in world-space coordinates

///////////////// END OF G L O B A L S //////////////////////////////////////////////////

// Fur simulations

/*
// Specifying shell geometries based on g_tipPos, g_furHeight, and g_numShells.
// You need to call this function whenver the shell needs to be updated
static void updateShellGeometry() {
    // TASK 1 and 3 TODO: finish this function as part of Task 1 and Task 3
}

// New glut timer call back that perform dynamics simulation
// every g_simulationsPerSecond times per second
static void hairsSimulationCallback(int dontCare) {

    // TASK 2 TODO: wrte dynamics simulation code here as part of TASK2

    ...

        // schedule this to get called again
        glutTimerFunc(1000 / g_simulationsPerSecond, hairsSimulationCallback, 0);
    glutPostRedisplay(); // signal redisplaying
}

// New function that initialize the dynamics simulation
static void initSimulation() {
    g_tipPos.resize(g_bunnyMesh.getNumVertices(), Cvec3(0));
    g_tipVelocity = g_tipPos;

    // TASK 1 TODO: initialize g_tipPos to "at-rest" hair tips in world coordinates

    ...

        // Starts hair tip simulation
        hairsSimulationCallback(0);
}
*/

//! Geometry primitives initialization
static void initGround() {
    int ibLen, vbLen;
    getPlaneVbIbLen(vbLen, ibLen);

    // Temporary storage for cube geometry
    std::vector<VertexPNTBX> vtx(vbLen);
    std::vector<unsigned short> idx(ibLen);

    makePlane(g_groundSize * 2, vtx.begin(), idx.begin());
    g_ground.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initCubes() {
  int ibLen, vbLen;
  getCubeVbIbLen(vbLen, ibLen);

  // Temporary storage for cube geometry
  std::vector<VertexPNTBX> vtx(vbLen);
  std::vector<unsigned short> idx(ibLen);

  // create the first cube
  makeCube(1, vtx.begin(), idx.begin());
  g_cube.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}

static void initSpheres() {
    int slices = 20;
    int stacks = 20;
    int ibLen, vbLen;
    getSphereVbIbLen(slices, stacks, vbLen, ibLen);

    // Temporary storage for sphere geometry
    std::vector<VertexPNTBX> vtx(vbLen);
    std::vector<unsigned short> idx(ibLen);

    // create a sphere for arcball visualization
    makeSphere(1.0, slices, stacks, vtx.begin(), idx.begin());
    g_sphere.reset(new SimpleIndexedGeometryPNTBX(&vtx[0], &idx[0], vbLen, ibLen));
}
//! End of Geometry primitives initialization
//!
// takes a projection matrix and send to the the shaders
inline void sendProjectionMatrix(Uniforms& uniforms, const Matrix4& projMatrix) {
    uniforms.put("uProjMatrix", projMatrix);
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

static void drawStuff(bool picking) {

    Uniforms uniforms;

    // build & send proj. matrix to vshader
    const Matrix4 projmat = makeProjectionMatrix();
    sendProjectionMatrix(uniforms, projmat);

    // use the RBT of current eye node as the eyeRbt
    const RigTForm eyeRbt = g_currentEyeNode->getRbt();
    const RigTForm invEyeRbt = inv(eyeRbt);

    const Cvec3 eyeLight1 = Cvec3(invEyeRbt * Cvec4(getPathAccumRbt(g_world, g_light1Node).getTranslation(), 1)); // g_light1 position in eye coordinates
    const Cvec3 eyeLight2 = Cvec3(invEyeRbt * Cvec4(getPathAccumRbt(g_world, g_light2Node).getTranslation(), 1)); // g_light2 position in eye coordinates

    uniforms.put("uLight", eyeLight1);
    uniforms.put("uLight2", eyeLight2);

    if (!picking) {
        Drawer drawer(invEyeRbt, uniforms);
        g_world->accept(drawer);

        RigTForm MVRigTForm;
        if (!g_isWorldSky) {
            // arcball rendering in normal situation
            MVRigTForm = invEyeRbt * getPathAccumRbt(g_world, g_currentPickedRbtNode);

            if (g_currentEyeNode != g_currentPickedRbtNode) {

                if (!((g_mouseLClickButton && g_mouseRClickButton) || g_mouseMClickButton)) {
                    // calculate scale if z-motion
                    double z = MVRigTForm.getTranslation()(2);
                    g_arcballScale = getScreenToEyeScale(z, g_frustFovY, g_windowHeight);
                }

                Matrix4 MVM = rigTFormToMatrix(MVRigTForm);
                double scale = g_arcballScale * g_arcballScreenRadius;
                Matrix4 scale_mat = Matrix4::makeScale(Cvec3(scale, scale, scale));

                MVM *= scale_mat;
                sendModelViewNormalMatrix(uniforms, MVM, normalMatrix(MVM));

                g_arcballMat->draw(*g_sphere, uniforms);
            }
        }
        else {
            // arcball rendering in world-sky frame
            MVRigTForm = invEyeRbt * g_world->getRbt();

            if (!((g_mouseLClickButton && g_mouseRClickButton) || g_mouseMClickButton)) {
                // calculate scale if z-motion
                double z = MVRigTForm.getTranslation()(2);
                g_arcballScale = getScreenToEyeScale(z, g_frustFovY, g_windowHeight);
            }

            Matrix4 MVM = rigTFormToMatrix(MVRigTForm);
            double scale = g_arcballScale * g_arcballScreenRadius;
            Matrix4 scale_mat = Matrix4::makeScale(Cvec3(scale, scale, scale));

            MVM *= scale_mat;
            sendModelViewNormalMatrix(uniforms, MVM, normalMatrix(MVM));

            g_arcballMat->draw(*g_sphere, uniforms);
        }
    }
    else {
        Picker picker(invEyeRbt, uniforms);

        g_overridingMaterial = g_pickingMat;
        g_world->accept(picker);
        g_overridingMaterial.reset();
        glFlush();

        g_currentPickedRbtNode = picker.getRbtNodeAtXY(g_mouseClickX, g_mouseClickY);

        if (g_currentPickedRbtNode == nullptr) {
            // if any of robot part is not selected, switch to ego motion
            g_currentPickedRbtNode = g_currentEyeNode;
        }
    }
}

/* GLUT callbacks */
static void animateTimerCallback(int ms) {
    if (g_playing) {
        float t = static_cast<float>(ms) / static_cast<float>(g_msBetweenKeyFrames);
        Animation::Frame interFrame = Animation::Frame();
        bool endReached = g_keyframes.interpolateKeyframes(t, interFrame);

        if (!endReached) {
            // update current scene using interpolated frame
            setSgRbtNodes(g_sceneRbtVector, interFrame);
            glutPostRedisplay();

            // register another timer callback
            glutTimerFunc(1000 / g_animationFramesPerSecond,
                animateTimerCallback,
                ms + 1000 / g_animationFramesPerSecond);
        }
        else {
            std::cout << "Animation playback is finished...\n";
            // when reached the end of keyframes, set (n-1)th frame
            // as the current frame
            std::list<Animation::Frame>::iterator last = --g_keyframes.end();
            g_keyframes.setCurrentKeyframeAs(last);
            g_playing = false;
        }
    }
}

static void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // clear framebuffer color&depth

  drawStuff(g_isPicking);

  if (!g_isPicking) {
      glutSwapBuffers();
  }

  else {
      g_isPicking = false;
  }

  checkGlErrors();
}

static void reshape(const int w, const int h) {
  g_windowWidth = w;
  g_windowHeight = h;
  glViewport(0, 0, w, h);
  g_arcballScreenRadius = 0.25 * std::min(g_windowWidth, g_windowHeight);
  cerr << "Size of window is now " << w << "x" << h << endl;
  updateFrustFovY();
  glutPostRedisplay();
}

static void motion(const int x, const int y) {

    // TODO: Flip rotation or translation when required

    const double dx = x - g_mouseClickX;
    const double dy = g_windowHeight - y - 1 - g_mouseClickY;

    RigTForm eyeRbt = getPathAccumRbt(g_world, g_currentEyeNode);
    RigTForm invEyeRbt = inv(eyeRbt);

    RigTForm m;

    // motion callbacks for normal situations
    if (!g_isWorldSky) {
        // if not ego motion, enable arcball
        if (g_currentEyeNode != g_currentPickedRbtNode) {
            // rotation when arcball is visible
            if (g_mouseLClickButton && !g_mouseRClickButton) {
                Quat Rotation = Quat();

                Cvec2 arcballScreenCoord = getScreenSpaceCoord((invEyeRbt * getPathAccumRbt(g_world, g_currentPickedRbtNode)).getTranslation(),
                    makeProjectionMatrix(), g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight);

                // calculate z coordinate of clicked points in screen coordinate
                int v1_x = (int)(g_mouseClickX - arcballScreenCoord(0));
                int v1_y = (int)(g_mouseClickY - arcballScreenCoord(1));
                int v1_z = getScreenZ(g_arcballScreenRadius, g_mouseClickX, g_mouseClickY, arcballScreenCoord);

                int v2_x = (int)(x - arcballScreenCoord(0));
                int v2_y = (int)(g_windowHeight - y - 1 - arcballScreenCoord(1));
                int v2_z = getScreenZ(g_arcballScreenRadius, x, g_windowHeight - y - 1, arcballScreenCoord);

                Cvec3 v1;
                Cvec3 v2;
                Cvec3 k;

                if (v1_z < 0 || v2_z < 0) {
                    // user points outside the arcball
                    v1 = normalize(Cvec3(v1_x, v1_y, 0));
                    v2 = normalize(Cvec3(v2_x, v2_y, 0));
                    k = cross(v1, v2);

                    Rotation = Quat(dot(v1, v2), k);
                }

                else {
                    v1 = normalize(Cvec3(v1_x, v1_y, v1_z));
                    v2 = normalize(Cvec3(v2_x, v2_y, v2_z));
                    k = cross(v1, v2);

                    Rotation = Quat(dot(v1, v2), k);
                }

                m = RigTForm(Rotation);
            }

            // translation when arcball is visible
            else {
                if (g_mouseRClickButton && !g_mouseLClickButton) {
                    // right click
                    m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * g_arcballScale);
                }

                else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {
                    // middle of both button click
                    m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * g_arcballScale);
                }
            }
        }

        else {
            // interface when arcball is NOT visible
            if (g_mouseLClickButton && !g_mouseRClickButton) {

                // left button down. rotation
                m = RigTForm::makeXRotation(dy) * RigTForm::makeYRotation(-dx);
            }
            else {
                if (g_mouseRClickButton && !g_mouseLClickButton) {
                    // right button clicked. translation on xy plane
                    m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * 0.01);
                }
                else {
                    m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * 0.01);
                }
            }
        }

        if (g_mouseClickDown) {
            // calculate auxiliary frame
            RigTForm AuxFrame = inv(getPathAccumRbt(g_world, g_currentPickedRbtNode, 1)) * RigTForm(getPathAccumRbt(g_world, g_currentPickedRbtNode).getTranslation(), eyeRbt.getRotation());      // apply transform to the object

            // apply transform
            g_currentPickedRbtNode->setRbt(doMtoOwrtA(m, g_currentPickedRbtNode->getRbt(), AuxFrame));

            glutPostRedisplay(); // we always redraw if we changed the scene
        }
    }

    // motion callback for world-sky frame
    else {
        if (g_mouseLClickButton && !g_mouseRClickButton) {
            Quat Rotation = Quat();

            Cvec2 arcballScreenCoord = getScreenSpaceCoord((invEyeRbt * g_world->getRbt()).getTranslation(),
                makeProjectionMatrix(), g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight);

            // calculate z coordinate of clicked points in screen coordinate
            int v1_x = (int)(g_mouseClickX - arcballScreenCoord(0));
            int v1_y = (int)(g_mouseClickY - arcballScreenCoord(1));
            int v1_z = getScreenZ(g_arcballScreenRadius, g_mouseClickX, g_mouseClickY, arcballScreenCoord);

            int v2_x = (int)(x - arcballScreenCoord(0));
            int v2_y = (int)(g_windowHeight - y - 1 - arcballScreenCoord(1));
            int v2_z = getScreenZ(g_arcballScreenRadius, x, g_windowHeight - y - 1, arcballScreenCoord);

            Cvec3 v1;
            Cvec3 v2;
            Cvec3 k;

            if (v1_z < 0 || v2_z < 0) {
                // user points outside the arcball
                v1 = normalize(Cvec3(v1_x, v1_y, 0));
                v2 = normalize(Cvec3(v2_x, v2_y, 0));
                k = cross(v1, v2);

                Rotation = Quat(dot(v1, v2), k);
            }

            else {
                v1 = normalize(Cvec3(v1_x, v1_y, v1_z));
                v2 = normalize(Cvec3(v2_x, v2_y, v2_z));
                k = cross(v1, v2);

                Rotation = Quat(dot(v1, v2), k);
            }

            m = RigTForm(Rotation);
        }

        // translation when arcball is visible
        else {
            if (g_mouseRClickButton && !g_mouseLClickButton) {
                // right click
                m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * g_arcballScale);
            }

            else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {
                // middle of both button click
                m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * g_arcballScale);
            }
        }

        if (g_mouseClickDown) {
            // calculate auxiliary frame
            RigTForm AuxFrame = makeMixedFrame(g_world->getRbt(), g_currentEyeNode->getRbt());

            g_skyNode->setRbt(doMtoOwrtA(inv(m), g_skyNode->getRbt(), AuxFrame));

            glutPostRedisplay(); // we always redraw if we changed the scene
        }
    }
  g_mouseClickX = x;
  g_mouseClickY = g_windowHeight - y - 1;
}

/* Helper functions for motions */
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

static void pick() {
    // We need to set the clear color to black, for pick rendering.
    // so let's save the clear color
    GLdouble clearColor[4];
    glGetDoublev(GL_COLOR_CLEAR_VALUE, clearColor);

    glClearColor(0, 0, 0, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawStuff(true);

    // Uncomment below and comment out the glutPostRedisplay in mouse(...) call back
    // to see result of the pick rendering pass
    // glutSwapBuffers();

    //Now set back the clear color
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

    checkGlErrors();
}

static void keyboard(const unsigned char key, const int x, const int y) {

    switch (key) {

    case 27:
        // quit application
        exit(0);                                  // ESC

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


    case 'v':
        // switch view point
        std::cout << "Pressed 'v'! Switching camera\n";
        g_isWorldSky = false;
        if (g_currentEyeNode == g_skyNode) {
            g_currentEyeNode = g_robot1Node;
            g_currentPickedRbtNode = g_robot1Node;
        }
        else if (g_currentEyeNode == g_robot1Node) {
            g_currentEyeNode = g_robot2Node;
            g_currentPickedRbtNode = g_robot2Node;
        }
        else {
            g_currentEyeNode = g_skyNode;
            g_currentPickedRbtNode = g_skyNode;
        }
        glutPostRedisplay();
        break;

    case 'm':
        // toggle world-sky frame if possible
        if (g_isWorldSky) {
            g_isWorldSky = false;
            glutPostRedisplay();
            break;
        }

        if (g_currentEyeNode != g_skyNode) {
            std::cout << "You should be in bird-eye view to switch to World-Sky frame\n";
        }
        else {
            g_isWorldSky = true;
        }
        glutPostRedisplay();
        break;

    case 's':
        // capture screen
        glFlush();
        writePpmScreenshot(g_windowWidth, g_windowHeight, "out.ppm");
        glutPostRedisplay();
        break;

    case 'p':
        // picking
        std::cout << "Pressed 'p'! ";
        std::cout << "Enabling picking... \n";
        // pick();
        g_isPicking = true;
        g_isWorldSky = false;
        break;

    case 'y':
    {
        // play animation
        if (!g_playing) {
            if (g_keyframes.size() < 4) {
                std::cerr << "We need at least 4 keyframes to play animation!\n";
                break;
            }
            g_playing = true;
            animateTimerCallback(0);
        }
        else {
            // stop playing animation
            g_playing = false;
            g_keyframes.sendCurrentKeyframeToScene(g_sceneRbtVector);
            glutPostRedisplay();
        }
        break;
    }

    case '+':
    {
        if (g_msBetweenKeyFrames >= 200) {
            g_msBetweenKeyFrames -= 100;
        }
        else {
            std::cout << "Time between each frame should be greater than 100!\n";
        }
        std::cout << "Time between each frame is now: " << g_msBetweenKeyFrames << "\n";
        break;
    }

    case '-':
    {
        g_msBetweenKeyFrames += 100;
        std::cout << "Time between each frame is now: " << g_msBetweenKeyFrames << "\n";
        break;
    }

    case 32:
        // copy current keyframe into the scene
        std::cout << "Copying current keyframe into the scene...\n";
        g_keyframes.sendCurrentKeyframeToScene(g_sceneRbtVector);
        glutPostRedisplay();
        break;

    case 'u':
    {
        // update the scene graph RBT data to the current keyframe
        // or, add a new keyframe if the keyframe list is empty
        Animation::Frame dumpedFrame = Animation::Frame();
        dumpFrame(g_sceneRbtVector, dumpedFrame);

        if (g_keyframes.empty()) {
            std::cout << "Current keyframe is undefined. Adding one...\n";
            g_keyframes.addNewKeyframe(dumpedFrame);
        }
        else {
            std::cout << "Updating current keyframe...\n";
            g_keyframes.updateCurrentKeyframe(dumpedFrame);
        }
        g_keyframes.printCurrentKeyframeIdx();
        break;
    }

    case 'd':
        // remove current keyframe from the list
        std::cout << "Removing current keyframe...\n";
        g_keyframes.removeCurrentKeyframe(g_sceneRbtVector);
        glutPostRedisplay();
        g_keyframes.printCurrentKeyframeIdx();
        break;


    case 'n':
    {
        // add new keyframe
        std::cout << "Adding new keyframe...\n";
        Animation::Frame dumpedFrame = Animation::Frame();
        dumpFrame(g_sceneRbtVector, dumpedFrame);
        g_keyframes.addNewKeyframe(dumpedFrame);
        g_keyframes.printCurrentKeyframeIdx();
        break;
    }

    case '>':
    {
        // advance to next frame
        g_keyframes.advanceFrame(g_sceneRbtVector);
        glutPostRedisplay();
        g_keyframes.printCurrentKeyframeIdx();
        break;
    }

    case '<':
    {
        // retreat to previous frame
        g_keyframes.retreatFrame(g_sceneRbtVector);
        glutPostRedisplay();
        g_keyframes.printCurrentKeyframeIdx();
        break;
    }

    case 'w':
    {
        // write current keyframe list to a file
        std::cout << "Writing current keyframe list...\n";
        std::string filename = "keyframe.txt";
        g_keyframes.exportKeyframeList(filename);
        break;
    }

    case 'i':
    {
        // read keyframe data from a file
        std::cout << "Reading keyframe list from the file...\n";
        std::string filename = "keyframe.txt";
        g_keyframes.importKeyframeList(filename);
        break;
    }
    }
}

// new  special keyboard callback, for arrow keys
static void specialKeyboard(const int key, const int x, const int y) {
    switch (key) {
    case GLUT_KEY_RIGHT:
        g_furHeight *= 1.05;
        cerr << "fur height = " << g_furHeight << std::endl;
        break;
    case GLUT_KEY_LEFT:
        g_furHeight /= 1.05;
        std::cerr << "fur height = " << g_furHeight << std::endl;
        break;
    case GLUT_KEY_UP:
        g_hairyness *= 1.05;
        cerr << "hairyness = " << g_hairyness << std::endl;
        break;
    case GLUT_KEY_DOWN:
        g_hairyness /= 1.05;
        cerr << "hairyness = " << g_hairyness << std::endl;
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
    glutCreateWindow("Assignment 9");                       // title the window

    glutDisplayFunc(display);                               // display rendering callback
    glutReshapeFunc(reshape);                               // window reshape callback
    glutMotionFunc(motion);                                 // mouse movement callback
    glutMouseFunc(mouse);                                   // mouse click callback
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);                       // special keyboard callback
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

static void initMaterials() {
    // Create some prototype materials
    Material diffuse("./shaders/basic-gl3.vshader", "./shaders/diffuse-gl3.fshader");
    Material specular("./shaders/basic-gl3.vshader", "./shaders/specular-gl3.fshader");
    Material solid("./shaders/basic-gl3.vshader", "./shaders/solid-gl3.fshader");

    // copy diffuse prototype and set red color
    g_redDiffuseMat.reset(new Material(diffuse));
    g_redDiffuseMat->getUniforms().put("uColor", Cvec3f(1, 0, 0));

    // copy diffuse prototype and set blue color
    g_blueDiffuseMat.reset(new Material(diffuse));
    g_blueDiffuseMat->getUniforms().put("uColor", Cvec3f(0, 0, 1));

    // normal mapping material
    g_bumpFloorMat.reset(new Material("./shaders/normal-gl3.vshader", "./shaders/normal-gl3.fshader"));
    g_bumpFloorMat->getUniforms().put("uTexColor", shared_ptr<ImageTexture>(new ImageTexture("Fieldstone.ppm", true)));
    g_bumpFloorMat->getUniforms().put("uTexNormal", shared_ptr<ImageTexture>(new ImageTexture("FieldstoneNormal.ppm", false)));

    // copy solid prototype, and set to wireframed rendering
    g_arcballMat.reset(new Material(solid));
    g_arcballMat->getUniforms().put("uColor", Cvec3f(0.27f, 0.82f, 0.35f));
    g_arcballMat->getRenderStates().polygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // copy solid prototype, and set to color white
    g_lightMat.reset(new Material(solid));
    g_lightMat->getUniforms().put("uColor", Cvec3f(1, 1, 1));

    // pick shader
    g_pickingMat.reset(new Material("./shaders/basic-gl3.vshader", "./shaders/pick-gl3.fshader"));

    // Assignment 8 -> Dynamic mesh deformation
    // copy specular prototype, and set to color purple
    g_purpleSpecularMat.reset(new Material(specular));
    g_purpleSpecularMat->getUniforms().put("uColor", Cvec3f(0.35f, 0.27f, 0.82f));

    // Assignment 9 -> Bunny Fur animation
    // bunny material
    g_bunnyMat.reset(new Material("./shaders/basic-gl3.vshader", "./shaders/bunny-gl3.fshader"));
    g_bunnyMat->getUniforms()
        .put("uColorAmbient", Cvec3f(0.45f, 0.3f, 0.3f))
        .put("uColorDiffuse", Cvec3f(0.2f, 0.2f, 0.2f));

    // bunny shell materials;
    std::shared_ptr<ImageTexture> shellTexture(new ImageTexture("shell.ppm", false)); // common shell texture

    // needs to enable repeating of texture coordinates
    shellTexture->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // eachy layer of the shell uses a different material, though the materials will share the
    // same shader files and some common uniforms. hence we create a prototype here, and will
    // copy from the prototype later
    Material bunnyShellMatPrototype("./shaders/bunny-shell-gl3.vshader", "./shaders/bunny-shell-gl3.fshader");
    bunnyShellMatPrototype.getUniforms().put("uTexShell", shellTexture);
    bunnyShellMatPrototype.getRenderStates()
        .blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) // set blending mode
        .enable(GL_BLEND) // enable blending
        .disable(GL_CULL_FACE); // disable culling

    // allocate array of materials
    g_bunnyShellMats.resize(g_numShells);
    for (int i = 0; i < g_numShells; ++i) {
        g_bunnyShellMats[i].reset(new Material(bunnyShellMatPrototype)); // copy from the prototype
        // but set a different exponent for blending transparency
        g_bunnyShellMats[i]->getUniforms().put("uAlphaExponent", 2.f + 5.f * float(i + 1) / g_numShells);
    }
}

static void initBunnyMeshes() {

    // load mesh file
    g_bunnyMesh.load("bunny.mesh");

    // reset geometry
    g_bunnyGeometry.reset(new SimpleGeometryPN());

    // TODO: Init the per vertex normal of g_bunnyMesh, using codes from asst7
    std::vector<VertexPN> vtx;

    // Bunny geometry should use smooth vector by default
    std::vector<int> vertexValence(g_bunnyMesh.getNumVertices());

    // Step 1. Zero out all normals at each vertex
    for (int i = 0; i < g_bunnyMesh.getNumVertices(); ++i) {
        g_bunnyMesh.getVertex(i).setNormal(Cvec3(0, 0, 0));
    }

    // Step 2. Iterate through the faces, accumulate normal to adjacent vertices
    for (int i = 0; i < g_bunnyMesh.getNumFaces(); ++i) {
        Mesh::Face face = g_bunnyMesh.getFace(i);

        for (int j = 0; j < face.getNumVertices(); ++j) {
            Mesh::Vertex currentVertex = face.getVertex(j);
            Cvec3 currentVertexNormal = currentVertex.getNormal();
            currentVertexNormal += face.getNormal();
            currentVertex.setNormal(currentVertexNormal);

            // increase the number of vertex valence
            vertexValence[currentVertex.getIndex()] += 1;
        }
    }

    // Step 3. Visit each vertex and divide normal by valence (averaging)
    for (int i = 0; i < g_bunnyMesh.getNumVertices(); ++i) {
        Mesh::Vertex currentVertex = g_bunnyMesh.getVertex(i);
        Cvec3 currentVertexNormal = currentVertex.getNormal();
        currentVertexNormal /= vertexValence[currentVertex.getIndex()];
        currentVertex.setNormal(currentVertexNormal);
    }

    // Iterate over faces, put associated vertex & normal in the vector
    for (int i = 0; i < g_bunnyMesh.getNumFaces(); ++i) {
        Mesh::Face face = g_bunnyMesh.getFace(i);

        // push triangle parameters
        vtx.push_back(VertexPN(face.getVertex(0).getPosition(), face.getVertex(0).getNormal()));
        vtx.push_back(VertexPN(face.getVertex(1).getPosition(), face.getVertex(1).getNormal()));
        vtx.push_back(VertexPN(face.getVertex(2).getPosition(), face.getVertex(2).getNormal()));
    }

    int vbLen = vtx.size();

    g_bunnyGeometry->upload(&vtx[0], vbLen);

    // Now allocate array of SimpleGeometryPNX to for shells, one per layer
    g_bunnyShellGeometries.resize(g_numShells);
    for (int i = 0; i < g_numShells; ++i) {
        g_bunnyShellGeometries[i].reset(new SimpleGeometryPNX());
    }
}

static void initGeometry() {
    initGround();
    initCubes();
    initSpheres();
    initBunnyMeshes();
}

static void constructRobot(shared_ptr<SgTransformNode> base, std::shared_ptr<Material> material) {

    const double ARM_LEN = 0.7;
    const double ARM_THICK = 0.25;
    const double LEG_LEN = 0.7;
    const double LEG_THICK = 0.25;
    const double TORSO_LEN = 1.5;
    const double TORSO_THICK = 0.25;
    const double TORSO_WIDTH = 1;
    const double HEAD_RADIUS = 0.4;

    const int NUM_JOINTS = 10;
    const int NUM_SHAPES = 10;

    struct JointDesc {
        int parent;
        float x, y, z;
    };

    JointDesc jointDesc[NUM_JOINTS] = {
        {-1}, // torso

        {0, 0, TORSO_LEN * 2 / 3, 0},  // head

        {0, TORSO_WIDTH / 2, TORSO_LEN / 2, 0}, // right shoulder
        {0, -TORSO_WIDTH / 2, TORSO_LEN / 2, 0}, // left shoulder

        {0, TORSO_WIDTH / 2 - 0.2, -TORSO_LEN / 2, 0}, // upper right leg
        {0, -TORSO_WIDTH / 2 + 0.2, -TORSO_LEN / 2, 0}, // upper left leg

        {2,  ARM_LEN, 0, 0}, // right elbow
        {3, -ARM_LEN, 0, 0}, // left elbow

        {4, 0, -LEG_LEN, 0}, // right knee
        {5, 0, -LEG_LEN, 0}  // left knee
    };

    struct ShapeDesc {
        int parentJointId;
        float x, y, z, sx, sy, sz;
        shared_ptr<Geometry> geometry;
    };

    ShapeDesc shapeDesc[NUM_SHAPES] = {
        {0, 0, 0, 0, TORSO_WIDTH, TORSO_LEN, TORSO_THICK, g_cube}, // torso

        {1, 0, HEAD_RADIUS, 0, HEAD_RADIUS, HEAD_RADIUS, HEAD_RADIUS, g_sphere},  // head

        {2, ARM_LEN / 2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // upper right arm (<- right shoulder)
        {3, -ARM_LEN / 2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube},  // upper left arm (<- left shoulder)

        {4, 0, -LEG_LEN / 2, 0, LEG_THICK, LEG_LEN, LEG_THICK, g_cube}, // upper right leg
        {5, 0, -LEG_LEN / 2, 0, LEG_THICK, LEG_LEN, LEG_THICK, g_cube}, // upper left leg

        {6, ARM_LEN / 2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube}, // lower right arm (<- right elbow)
        {7, -ARM_LEN / 2, 0, 0, ARM_LEN, ARM_THICK, ARM_THICK, g_cube},  // lower left arm (<- left elbow)

        {8, 0, -LEG_LEN / 2, 0, LEG_THICK, LEG_LEN, LEG_THICK, g_cube},  // lower right leg (<- right knee)
        {9, 0, -LEG_LEN / 2, 0, LEG_THICK, LEG_LEN, LEG_THICK, g_cube}  // lower left leg (<- left knee)
    };

    shared_ptr<SgTransformNode> jointNodes[NUM_JOINTS];

    for (int i = 0; i < NUM_JOINTS; ++i) {
        if (jointDesc[i].parent == -1)
            jointNodes[i] = base;
        else {
            jointNodes[i].reset(new SgRbtNode(RigTForm(Cvec3(jointDesc[i].x, jointDesc[i].y, jointDesc[i].z))));
            jointNodes[jointDesc[i].parent]->addChild(jointNodes[i]);
        }
    }
    for (int i = 0; i < NUM_SHAPES; ++i) {
        shared_ptr<MyShapeNode> shape(
            new MyShapeNode(shapeDesc[i].geometry,
                material,
                Cvec3(shapeDesc[i].x, shapeDesc[i].y, shapeDesc[i].z),
                Cvec3(0, 0, 0),
                Cvec3(shapeDesc[i].sx, shapeDesc[i].sy, shapeDesc[i].sz)));
        jointNodes[shapeDesc[i].parentJointId]->addChild(shape);
    }
}

static void initScene() {
    g_world.reset(new SgRootNode());

    g_skyNode.reset(new SgRbtNode(RigTForm(Cvec3(0.0, 0.25, 4.0))));

    g_groundNode.reset(new SgRbtNode());
    g_groundNode->addChild(shared_ptr<MyShapeNode>(
        new MyShapeNode(g_ground, g_bumpFloorMat, Cvec3(0, g_groundY, 0))));

    g_light1Node.reset(new SgRbtNode(RigTForm(Cvec3(2.0, 3.0, 14.0))));
    g_light1Node->addChild(std::shared_ptr<MyShapeNode>(
        new MyShapeNode(g_sphere, g_lightMat, Cvec3(0, 0, 0))));

    g_light2Node.reset(new SgRbtNode(RigTForm(Cvec3(-2, -3.0, -5.0))));
    g_light2Node->addChild(std::shared_ptr<MyShapeNode>(
        new MyShapeNode(g_sphere, g_lightMat, Cvec3(0, 0, 0))));

    // initialize bunnyNode
    g_bunnyNode.reset(new SgRbtNode());
    g_bunnyNode->addChild(shared_ptr<MyShapeNode>(
        new MyShapeNode(g_bunnyGeometry, g_bunnyMat)));

    // add each shell as shape node
    for (int i = 0; i < g_numShells; ++i) {
        g_bunnyNode->addChild(shared_ptr<MyShapeNode>(
            new MyShapeNode(g_bunnyShellGeometries[i], g_bunnyShellMats[i])));
    }

    // from this point, calling g_bunnyShellGeometries[i]->reset(...) will change the
    // geometry of the ith layer of shell that gets drawn

    g_robot1Node.reset(new SgRbtNode(RigTForm(Cvec3(-10, 1, 0))));
    g_robot2Node.reset(new SgRbtNode(RigTForm(Cvec3(10, 1, 0))));

    constructRobot(g_robot1Node, g_redDiffuseMat); // a Red robot
    constructRobot(g_robot2Node, g_blueDiffuseMat); // a Blue robot

    // Add all nodes under the scene root
    g_world->addChild(g_skyNode);
    g_world->addChild(g_groundNode);
    g_world->addChild(g_light1Node);
    g_world->addChild(g_light2Node);
    // g_world->addChild(g_dynamicCubeNode);
    g_world->addChild(g_bunnyNode);
    g_world->addChild(g_robot1Node);
    g_world->addChild(g_robot2Node);

    // initially set eye and object
    g_currentEyeNode = g_skyNode;
    g_currentPickedRbtNode = g_skyNode;

    // dump current scene to a vector
    dumpSgRbtNodes(g_world, g_sceneRbtVector);
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
        initMaterials();
        initGeometry();
        initScene();
        /*
        * Artifact of assignment 8
        glutTimerFunc(1000 / g_animationFramesPerSecond, randomScaleTimerCallback, 0);
        */
        // initSimulation();
        glutMainLoop();
        return 0;
    }
    catch (const runtime_error& e) {
        cout << "Exception caught: " << e.what() << endl;
        return -1;
    }
}

/* End of main program routine */