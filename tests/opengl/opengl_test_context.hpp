#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

struct OpenGLTestContext
{
    public:
    OpenGLTestContext() = default;
    int initialize();
    void deinitialize();
    void handleEvents();

    Display* getDisplay();
    Window& getWindow();

    ~OpenGLTestContext()
    {
        deinitialize();
    }
    private:
        Display* display = nullptr;
        Window window;
        Screen* screen = nullptr;
        int screenId = 0;
        XEvent ev;
        GLXContext context;
        XVisualInfo* visual = nullptr;
        XSetWindowAttributes windowAttribs;
};


