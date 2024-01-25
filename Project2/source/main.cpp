#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyGL.h"
#include <iostream>

using namespace std;

int num_vertices;
GLuint VAO;
float rot_x = -90.0f;
float rot_y = 0.0f;
float camera_distance = 50.0f;
float lastX = 400, lastY = 300;
cy::GLSLProgram prog;
bool leftButtonPressed = false;
cy::TriMesh mesh;
bool orthogonal_projection_on = false;

cy::Matrix4f getOrthographicMatrix(float left, float right, float bottom, float top, float near, float far) {
        cy::Matrix4f proj = cy::Matrix4f(2.0f / (right - left), 0, 0, -(right + left) / (right - left),
        0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
        0, 0, -2.0f / (far - near),  -(far + near) / (far - near),
        0, 0, 0, 1);

        return proj;
}

void display() {
    // set uniforms
    cy::Matrix4f mvpMatrix = cy::Matrix4f(1.0);
    
     // Calculate the bounding box
    mesh.ComputeBoundingBox();
    cy::Vec3f center = (mesh.GetBoundMin() + mesh.GetBoundMax()) * 0.5f;

    //add two rotations to model matrix
    cy::Matrix4f model = cy::Matrix4f::RotationX(rot_x * 3.14 /180.0) * cy::Matrix4f::RotationY(rot_y * 3.14 /180.0);
    // Adjust the model transformation matrix to center the object
    model *= cy::Matrix4f::Translation(-center); 
    
    cy::Matrix4f view = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, -camera_distance));
    cy::Matrix4f proj = cy::Matrix4f(1.0);
    if (!orthogonal_projection_on) {
        proj *= cy::Matrix4f::Perspective(40 * 3.14 /180.0, 800.0/600.0, 0.1f, 1000.0f);
    } else {
        float scale_factor = 500.0f/camera_distance;
        proj*= getOrthographicMatrix(-scale_factor, scale_factor, -scale_factor, scale_factor, 0.1f, 1500.0f);
    }
    mvpMatrix = proj * view * model * mvpMatrix;
    prog["mvp"] = mvpMatrix;

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
    } else if (key == 'P' || key == 'p') {
        orthogonal_projection_on = !orthogonal_projection_on;
        if (orthogonal_projection_on) {
            cout << "Switched to orthogonal projection." << endl;
        } else {
            cout << "Switched to perspective projection." << endl;
        }
        glutPostRedisplay();
    }
}

void specialKeyboard(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_F6:
            // Reload shaders when F6 key is pressed
            prog.BuildFiles("vs.txt", "fs.txt");
            prog.Bind();
            cout << "Shaders recompiled successfully." << endl;
            glutPostRedisplay();
            break;
    }
}

void handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        leftButtonPressed = (state == GLUT_DOWN);
    }

    // Update last mouse position
    lastX = x;
    lastY = y;
}

void mouseMotion(int x, int y) {
    float xoffset = x - lastX;
    float yoffset = lastY - y; // reversed since y-coordinates range from bottom to top
    lastX = x;
    lastY = y;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    if (leftButtonPressed) {
        rot_x +=yoffset;
        rot_y +=xoffset;
    } else {
        camera_distance -= yoffset;
    }

    glutPostRedisplay();
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
    glutMouseFunc(handleMouse);
    glutMotionFunc(mouseMotion);
    glutSpecialFunc(specialKeyboard);


    // load model
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
    prog.BuildFiles("vs.txt", "fs.txt");
    prog.Bind();


    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}