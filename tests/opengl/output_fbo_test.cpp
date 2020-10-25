#include <iostream>
#include "opengl_test_context.hpp"

#include "pipeline/output_fbo.hpp"

int main(int argc, char** argv) {
    OpenGLTestContext windowSystem;
    windowSystem.initialize();

    ve::OutputFBO fbo;
    fbo.initialize();

    fbo.renderToBackbuffer();

    // Set GL Sample stuff
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    /*    // Enter message loop
    while (true) {
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
                glColor3f(  1.0f,  0.0f, 0.0f);
                glVertex3f( 0.0f, -1.0f, 0.0f);
                glColor3f(  0.0f,  1.0f, 0.0f);
                glVertex3f(-1.0f,  1.0f, 0.0f);
                glColor3f(  0.0f,  0.0f, 1.0f);
                glVertex3f( 1.0f,  1.0f, 0.0f);
        glEnd();

        // Present frame
        glXSwapBuffers(windowSystem.getDisplay(), windowSystem.getWindow());
    }
    */
    return 0;
}

