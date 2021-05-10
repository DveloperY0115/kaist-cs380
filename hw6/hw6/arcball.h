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

