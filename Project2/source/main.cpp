#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyGL.h"

using namespace std;

int num_vertices;
GLuint VAO;


void display() {
    // Your rendering code goes here

    glClear(GL_COLOR_BUFFER_BIT);
    // Draw your graphics here
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, num_vertices);
    glBindVertexArray(0);
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {  // ASCII value for the Esc key
        glutLeaveMainLoop();
    }
}

void idle() {
    // hue += 0.1f;

    // hue = fmod(hue, 360.0f);

    // // Trigger a redraw
    // glutPostRedisplay();
}

void loadModel(int argc, char** argv, cy::TriMesh & mesh) {
    char * modelName;

    // load model
    if (argc<1) {
        cout << "No model given. Specify model name in cmd-line arg." << endl;
        exit(0);
    } else {
        modelName = argv[1];
        cout << "Loading " << modelName << "..." << endl;
    }

    bool success = mesh.LoadFromFileObj(modelName);
    if (!success) {
        cout << "Model loading failed." << endl;
        exit(0);
    } else {
        cout << "Loaded model successfully." << endl;
        num_vertices = mesh.NV();
    }
}



int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);

    // Set OpenGL version and profile
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    // Set up a double-buffered window with RGBA color
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    // some default settings
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Create a window with a title
    glutCreateWindow("Hello Window");

    // Initialize GLEW
    glewInit();

    // Set up callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);





    // load model
    cy::TriMesh mesh;
    loadModel(argc, argv, mesh);

    // set up VAO and VBO
    glGenVertexArrays(1, &VAO); 
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*num_vertices, &mesh.V(0), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


    // link shaders
    cy::GLSLProgram prog;
    prog.BuildFiles("vs.txt", "fs.txt");

    // set uniforms
    cy::Matrix4f mvpMatrix = cy::Matrix4f(0.05);
    prog["mvp"] = mvpMatrix;
    prog.Bind();


    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}