#include <GL/freeglut.h>
#include <cmath>

using namespace std;

GLfloat hue = 0.0f;


void display() {
    // Convert HSV to RGB
    GLfloat saturation = 1.0f;
    GLfloat value = 1.0f;

    GLfloat chroma = value * saturation;
    GLfloat hue_prime = fmod(hue / 60.0f, 6.0f);
    GLfloat x = chroma * (1 - fabs(fmod(hue_prime, 2) - 1));

    GLfloat r, g, b;
    if (0 <= hue_prime && hue_prime < 1) {
        r = chroma;
        g = x;
        b = 0;
    } else if (1 <= hue_prime && hue_prime < 2) {
        r = x;
        g = chroma;
        b = 0;
    } else if (2 <= hue_prime && hue_prime < 3) {
        r = 0;
        g = chroma;
        b = x;
    } else if (3 <= hue_prime && hue_prime < 4) {
        r = 0;
        g = x;
        b = chroma;
    } else if (4 <= hue_prime && hue_prime < 5) {
        r = x;
        g = 0;
        b = chroma;
    } else {
        r = chroma;
        g = 0;
        b = x;
    }

    GLfloat m = value - chroma;
    r += m;
    g += m;
    b += m;

    // Your rendering code goes here
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw your graphics here
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  // ASCII value for the Esc key
        glutLeaveMainLoop();
    }
}

void idle() {
    hue += 0.1f;

    hue = fmod(hue, 360.0f);

    // Trigger a redraw
    glutPostRedisplay();
}



int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);

    // Set up a double-buffered window with RGBA color
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // Set the window size and position
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);

    // Create a window with a title
    glutCreateWindow("Hello Window");

    // Set up callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}