#ifndef ARCBALL_H
#define ARCBALL_H

#include <iostream>
#include "cvec.h"
#include "matrix4.h"
#include "rigtform.h"
#include "geometry.h"

// forward declaration
inline Cvec2 getScreenSpaceCoord(const Cvec3& p, const Matrix4& projection,
                                 double frustNear, double frustFovY,
                                 int screenWidth, int screenHeight);
inline double getScreenToEyeScale(double z, double frustFovY, int screenHeight);
inline int getScreenZ(double screenRadius, int x, int y, Cvec2 centerCoord);

class Arcball {
public:
    Arcball() {
        return;
    }

    Arcball(std::shared_ptr<Geometry> geometry,
        Cvec3 color,
        double screenRadius,
        double scale) {
        ArcballGeometry = geometry;
        ArcballColor = color;
        ArcballScreenRadius = screenRadius;
        ArcballScale = scale;
    }

    /*
    * drawArcball
    */
    void drawArcball(const ShaderState& curSS) {

        Matrix4 MVM, NMVM;

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


        MVM = RigTFormToMatrix(ArcballMVRbt);

        double scale = ArcballScale * ArcballScreenRadius;
        Matrix4 scale_mat = Matrix4::makeScale(Cvec3(scale, scale, scale));

        MVM *= scale_mat;
        NMVM = normalMatrix(MVM);
        sendModelViewNormalMatrix(curSS, MVM, NMVM);

        safe_glUniform3f(curSS.h_uColor, ArcballColor(0), ArcballColor(1), ArcballColor(2));
        ArcballGeometry->draw(curSS);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // end wireframe mode
    }

    /*
    * isVisible
    * 
    * Arcball is visible when
    * 1) Cube is controlled but not ego motion
    * 2) World-Sky frame is controlled
    * 
    * Most importantly, the z coordinate of the arcball must be smaller than 0
    */
    bool isVisible() {
        return ArcballMVRbt.getTranslation()(2) < -CS175_EPS;
    }

    double getScreenRadius() {
        return ArcballScreenRadius;
    }

    RigTForm getArcballMVRbt() {
        return ArcballMVRbt;
    }

    double getArcballScale() {
        return ArcballScale;
    }

    void updateScreenRadius(const double new_radius) {
        ArcballScreenRadius = new_radius;
    }

    void updateArcballMVRbt(const RigTForm new_Rbt) {
        ArcballMVRbt = new_Rbt;
    }

    void updateArcballScale(const double new_scale) {
        ArcballScale = new_scale;
    }

private:
    std::shared_ptr<Geometry> ArcballGeometry;
    RigTForm ArcballMVRbt;
    Cvec3 ArcballColor;
    double ArcballScreenRadius;
    double ArcballScale;
};


// Return the screen space projection in terms of pixels of a 3d point
// given in eye-frame coordinates. 
//
// Ideally you should never call this for a point behind the Z=0 plane,
// sinch such a point wouldn't be visible.  
//
// But if you do pass in a point behind Z=0 plane, we'll just
// print a warning, and return the center of the screen.
inline Cvec2 getScreenSpaceCoord(const Cvec3& p,
                                 const Matrix4& projection,
                                 double frustNear, double frustFovY,
                                 int screenWidth, int screenHeight) {
    if (p[2] > -CS175_EPS) {
        std::cerr << "WARNING: getScreenSpaceCoord of a point near or behind Z=0 plane. Returning screen-center instead." << std::endl;
        return Cvec2((screenWidth-1)/2.0, (screenHeight-1)/2.0);
    }
    
    Cvec4 q = projection * Cvec4(p, 1);
    Cvec3 clipCoord = Cvec3(q) / q[3];
    return Cvec2(clipCoord[0] * screenWidth / 2.0 + (screenWidth - 1)/2.0,
               clipCoord[1] * screenHeight / 2.0 + (screenHeight - 1)/2.0);
}

// Return the scale between 1 unit in screen pixels and 1 unit in the eye-frame
// (or world-frame, since we always use rigid transformations to represent one
// frame with resepec to another frame)
//
// Ideally you should never call this using a z behind the Z=0 plane,
// sinch such a point wouldn't be visible.  
//
// But if you do pass in a point behind Z=0 plane, we'll just
// print a warning, and return 1
inline double getScreenToEyeScale(double z, double frustFovY, int screenHeight) {
  
    if (z > -CS175_EPS) {
    std::cerr << "WARNING: getScreenToEyeScale on z near or behind Z=0 plane. Returning 1 instead." << std::endl;
    return 1;
  }

  return -(z * tan(frustFovY * CS175_PI/360.0)) * 2 / screenHeight;
}

// Calculate the z coordinate of the point of interaction in screen coordinate system
// Clamp the z value if necessary
inline int getScreenZ(double screenRadius, int x, int y, Cvec2 centerCoord) {
    /*
    * ScreenRadius: Radius of the arcball on screen
    * x: X-coordinate of the click
    * y: Y-coordinate of the click
    * center_coord: Screen space coordinate of the center of the arcball
    /*
    * 
    if (std::pow(ScreenRadius, 2) < std::pow(x - center_coord[0], 2) + std::pow(y - center_coord[1], 2)) {
        // clamp if necessary
        std::cout << "You're trying to move the ball without even touching it!\n";
        exit(-1);
    }
    */   

    if (std::pow(screenRadius, 2) < std::pow(x - centerCoord[0], 2) + std::pow(y - centerCoord[1], 2)) {
        // mouse pointer is outside the arcball
        // behavior -> rotate along z axis
        return -1;
    }

    else {
        // mouse pointer is inside the arcball
        return (int)std::sqrt(std::pow(screenRadius, 2) - std::pow(x - centerCoord[0], 2) - std::pow(y - centerCoord[1], 2));
    }
}
#endif

