#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>
#include <cmath>
#include <unistd.h>

using namespace std;

// I used ChatGPT to discuss C++/X11 concepts, implementation planning,
// testing ideas, and code review for this assignment.

struct Point3D
{
    double x;
    double y;
    double z;
};

struct Point2D
{
    int x;
    int y;
    bool visible;
};

const double CAMERA_X = 5.0;
const double CAMERA_Y = 7.0;
const double CAMERA_Z = 20.0;

const double FOCAL_LENGTH = 700.0;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const double CAMERA_YAW =
    atan2(CAMERA_X, CAMERA_Z) * 180.0 / M_PI;

const double CAMERA_PITCH =
    atan2(
        CAMERA_Y,
        sqrt(CAMERA_X * CAMERA_X + CAMERA_Z * CAMERA_Z)
    ) * 180.0 / M_PI;

Point3D rotate_x(Point3D p, double angle)
{
    double radians = angle * M_PI / 180.0;

    Point3D rotated;

    rotated.x = p.x;
    rotated.y = p.y * cos(radians) - p.z * sin(radians);
    rotated.z = p.y * sin(radians) + p.z * cos(radians);

    return rotated;
}

Point3D rotate_y(Point3D p, double angle)
{
    double radians = angle * M_PI / 180.0;

    Point3D rotated;

    rotated.x = p.x * cos(radians) - p.z * sin(radians);
    rotated.y = p.y;
    rotated.z = p.x * sin(radians) + p.z * cos(radians);

    return rotated;
}

Point3D rotate_z(Point3D p, double angle)
{
    double radians = angle * M_PI / 180.0;

    Point3D rotated;

    rotated.x = p.x * cos(radians) - p.y * sin(radians);
    rotated.y = p.x * sin(radians) + p.y * cos(radians);
    rotated.z = p.z;

    return rotated;
}


Point2D project_point(Point3D p, double headAngle)
{
    Point3D cameraPoint = {
        p.x - CAMERA_X,
        p.y - CAMERA_Y,
        p.z - CAMERA_Z
    };

    // First point the camera toward the origin.
    // headAngle then adds the user's left/right head movement.
    cameraPoint = rotate_y(
        cameraPoint,
        CAMERA_YAW + headAngle
    );

    cameraPoint = rotate_x(
        cameraPoint,
        CAMERA_PITCH
    );

    double depth = -cameraPoint.z;

    // Points on or behind the camera plane cannot be projected safely.
    if (depth <= 0.1)
    {
        return {0, 0, false};
    }

    double projectedX =
        cameraPoint.x * FOCAL_LENGTH / depth;

    double projectedY =
        cameraPoint.y * FOCAL_LENGTH / depth;

    Point2D screenPoint;

    screenPoint.x =
        projectedX + SCREEN_WIDTH / 2;

    // Screen coordinates increase downward, unlike the virtual y-axis.
    screenPoint.y =
        -projectedY + SCREEN_HEIGHT / 2;

    screenPoint.visible = true;

    return screenPoint;
}

const int CUBE_POINT_COUNT = 8;

Point3D cube[CUBE_POINT_COUNT] = {
    {-2, -2, -2},
    { 2, -2, -2},
    { 2, -2,  2},
    {-2, -2,  2},

    {-2,  2, -2},
    { 2,  2, -2},
    { 2,  2,  2},
    {-2,  2,  2}
};

const int CUBE_EDGE_COUNT = 12;

int cubeEdges[CUBE_EDGE_COUNT][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},

    {4, 5},
    {5, 6},
    {6, 7},
    {7, 4},

    {0, 4},
    {1, 5},
    {2, 6},
    {3, 7}
};

int main()
{
    Display *display = XOpenDisplay(nullptr);

    if (display == nullptr)
    {
        cout << "Could not open X display." << endl;
        return 1;
    }

    int screen = DefaultScreen(display);

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        1,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );

    GC gc = XCreateGC(display, window, 0, nullptr);

    XSetForeground(display, gc, BlackPixel(display, screen));
    XSetLineAttributes(display, gc, 2, LineSolid, CapButt, JoinMiter);

    XSelectInput(display, window, ExposureMask | KeyPressMask);

    XMapWindow(display, window);

    bool running = true;
    double headAngle = 0.0;
    double shapeAngle = 0.0;

    while (running)
    {
        // Process any keyboard/window events that are waiting.
        while (XPending(display) > 0)
        {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == KeyPress)
            {
                KeySym key = XLookupKeysym(&event.xkey, 0);

                if (key == XK_Escape)
                {
                    running = false;
                }
                else if (key == XK_Left)
                {
                    // Left arrow turns the viewer's head to the right.
                    headAngle -= 5.0;
                }
                else if (key == XK_Right)
                {
                    // Right arrow turns the viewer's head to the left.
                    headAngle += 5.0;
                }

                // Keep the angle bounded instead of letting it grow forever.
                if (headAngle >= 360.0)
                {
                    headAngle -= 360.0;
                }

                if (headAngle <= -360.0)
                {
                    headAngle += 360.0;
                }
            }
        }

        // Clear the previous frame before drawing the next one.
        XClearWindow(display, window);

        // --------------------
        // Draw coordinate axes
        // --------------------

        Point3D xStart = {-10, 0, 0};
        Point3D xEnd   = {10, 0, 0};

        Point3D yStart = {0, -10, 0};
        Point3D yEnd   = {0, 10, 0};

        Point3D zStart = {0, 0, -10};
        Point3D zEnd   = {0, 0, 10};

        Point2D xs = project_point(xStart, headAngle);
        Point2D xe = project_point(xEnd, headAngle);

        Point2D ys = project_point(yStart, headAngle);
        Point2D ye = project_point(yEnd, headAngle);

        Point2D zs = project_point(zStart, headAngle);
        Point2D ze = project_point(zEnd, headAngle);

        if (xs.visible && xe.visible)
        {
            XDrawLine(display, window, gc, xs.x, xs.y, xe.x, xe.y);
        }

        if (ys.visible && ye.visible)
        {
            XDrawLine(display, window, gc, ys.x, ys.y, ye.x, ye.y);
        }

        if (zs.visible && ze.visible)
        {
            XDrawLine(display, window, gc, zs.x, zs.y, ze.x, ze.y);
        }

        // --------------------
        // Draw rotating cube
        // --------------------

        Point2D projectedCube[CUBE_POINT_COUNT];

        for (int i = 0; i < CUBE_POINT_COUNT; i++)
        {
            Point3D rotatedPoint = cube[i];

            rotatedPoint = rotate_x(rotatedPoint, shapeAngle);
            rotatedPoint = rotate_y(rotatedPoint, shapeAngle);


            projectedCube[i] =
                project_point(rotatedPoint, headAngle);
        }

        for (int i = 0; i < CUBE_EDGE_COUNT; i++)
        {
            int start = cubeEdges[i][0];
            int end = cubeEdges[i][1];

            if (projectedCube[start].visible &&
                projectedCube[end].visible)
            {
                XDrawLine(
                    display,
                    window,
                    gc,
                    projectedCube[start].x,
                    projectedCube[start].y,
                    projectedCube[end].x,
                    projectedCube[end].y
                );
            }
        }

        XFlush(display);

        // Advance the object's rotation for the next frame.
        shapeAngle += 1.0;

        if (shapeAngle >= 360.0)
        {
            shapeAngle -= 360.0;
        }

        // About 16.7 ms per frame gives approximately 60 frames per second.
        usleep(16667);
    }

    XDestroyWindow(display, window);
    XFreeGC(display, gc);
    XCloseDisplay(display);

    return 0;
}