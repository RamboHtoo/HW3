#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>

using namespace std;

// I used ChatGPT to discuss C++/X11 concepts, implementation planning,
// testing ideas, and code review for this assignment.

struct Point3D
{
    double x;
    double y;
    double z;
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


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

    while (running)
    {
        XEvent event;

        XNextEvent(display, &event);

        if (event.type == Expose)
        {
            cout << "Expose event" << endl;

            int centerX = SCREEN_WIDTH / 2;
            int centerY = SCREEN_HEIGHT / 2;

            XDrawLine(
                display,
                window,
                gc,
                0,
                centerY,
                SCREEN_WIDTH,
                centerY
            );

            XDrawLine(
                display,
                window,
                gc,
                centerX,
                0,
                centerX,
                SCREEN_HEIGHT
            );

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
                cout << "Turn head RIGHT" << endl;
            }
            else if (key == XK_Right)
            {
                cout << "Turn head LEFT" << endl;
            }
        }
    }

    XDestroyWindow(display, window);
    XFreeGC(display, gc);
    XCloseDisplay(display);

    return 0;
}