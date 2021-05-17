static RigTForm ArcballInterfaceRotation(const int x, const int y) {
    // rotation when arcball is visible
    Quat Rotation = Quat();

    RigTForm eyeRbt = g_VPState.getCurrentEyeRbt();
    RigTForm invEyeRbt = inv(eyeRbt);
    Cvec3 CenterEyeCoord = Cvec3();

    CenterEyeCoord = (invEyeRbt * g_VPState.getCurrentObjRbt()).getTranslation();

    Cvec2 center_screen_coord = getScreenSpaceCoord(CenterEyeCoord, makeProjectionMatrix(),
        g_frustNear, g_frustFovY, g_windowWidth, g_windowHeight);

    // calculate z coordinate of clicked points in screen coordinate

    int v1_x = (int)(g_mouseClickX - center_screen_coord(0));
    int v1_y = (int)(g_mouseClickY - center_screen_coord(1));
    int v1_z = getScreenZ(g_arcball.getScreenRadius(), g_mouseClickX, g_mouseClickY, center_screen_coord);

    // !!!!! Caution: Flip y before using it !!!!! -> TODO: implement function for better readability
    int v2_x = (int)(x - center_screen_coord(0));
    int v2_y = (int)(g_windowHeight - y - 1 - center_screen_coord(1));
    int v2_z = getScreenZ(g_arcball.getScreenRadius(), x, g_windowHeight - y - 1, center_screen_coord);

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

    /*
    if (g_VPState.getCurrentEyeNode() == g_skyNode && g_currentPickedRbtNode == g_world) {
        Rotation = inv(Rotation);
    }
    */

    return RigTForm(Rotation);
}

static RigTForm ArcballInterfaceTranslation(const int x, const int y) {

    RigTForm m;
    const double dx = x - g_mouseClickX;
    const double dy = g_windowHeight - y - 1 - g_mouseClickY;

    if (g_mouseRClickButton && !g_mouseLClickButton) { // right button down?
        switch (g_VPState.getAuxFrameDescriptor()) {
        case 1:
            // default behavior
            m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * g_arcball.getArcballScale());
            break;
        case 2:
            // invert sign of rotation and translation
            m = RigTForm::makeTranslation(-Cvec3(dx, dy, 0) * g_arcball.getArcballScale());
            break;

        case 3:
            // invert sign of rotation only
            m = RigTForm::makeTranslation(Cvec3(dx, dy, 0) * g_arcball.getArcballScale());
            break;
        }
    }
    else if (g_mouseMClickButton || (g_mouseLClickButton && g_mouseRClickButton)) {  // middle or (left and right) button down?
        switch (g_VPState.getAuxFrameDescriptor()) {
        case 1:
            // default behavior
            m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * g_arcball.getArcballScale());
            break;
        case 2:
            // invert sign of rotation and translation
            m = RigTForm::makeTranslation(-Cvec3(0, 0, -dy) * g_arcball.getArcballScale());
            break;
        case 3:
            // invert sign of rotation only
            m = RigTForm::makeTranslation(Cvec3(0, 0, -dy) * g_arcball.getArcballScale());
            break;
        }
    }

    return m;
}

static RigTForm DefaultInterfaceRotation(const int x, const int y) {

    RigTForm m;

    const double dx = x - g_mouseClickX;
    const double dy = g_windowHeight - y - 1 - g_mouseClickY;

    if (g_VPState.getCurrentEyeNode() != g_currentPickedRbtNode) {
        // default behavior
        m = RigTForm::makeXRotation(dy) * RigTForm::makeYRotation(dx);
    }

    else if (g_VPState.getCurrentEyeNode() == g_skyNode)
        // invert sign of rotation and translation
        m = RigTForm::makeXRotation(dy) * RigTForm::makeYRotation(-dx);

    else {
        // invert sign of rotation only
        m = RigTForm::makeXRotation(dy) * RigTForm::makeYRotation(-dx);
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
