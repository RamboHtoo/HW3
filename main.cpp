#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>
#include <cmath>

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

    while (running)
    {
        XEvent event;

        XNextEvent(display, &event);

        if (event.type == Expose)
        {
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

            XFlush(display);
        }
        else if (event.type == KeyPress)
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

            if (headAngle >= 360.0)
            {
                headAngle -= 360.0;
            }

            if (headAngle <= -360.0)
            {
                headAngle += 360.0;
            }

            XClearArea(
                display,
                window,
                0, 0,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                True
            );
        }
        else if (key == XK_Right)
        {
            // Right arrow turns the viewer's head to the left.
            headAngle += 5.0;

            if (headAngle >= 360.0)
            {
                headAngle -= 360.0;
            }

            if (headAngle <= -360.0)
            {
                headAngle += 360.0;
            }

            XClearArea(
                display,
                window,
                0, 0,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                True
            );
        }
        }
    }

    XDestroyWindow(display, window);
    XFreeGC(display, gc);
    XCloseDisplay(display);

    return 0;
}