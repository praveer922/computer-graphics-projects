#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <memory>
#include "PredefinedModels.h"


using namespace std;

namespace Init {
    float lastX = 400, lastY = 300;
    bool controlKeyPressed = false;
    bool leftButtonPressed = false;
    Camera* cameraPtr;
    shared_ptr<Object> modelObjPtr;
    shared_ptr<LightCubeObject> lightCubeObjPtr;

    void initGL(const char *name, int argc, char** argv) {
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


        // Create a window with a title
        glutCreateWindow(name);

        // Initialize GLEW
        glewInit();
        glEnable(GL_DEPTH_TEST);  
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    }

    void processInputModel(int argc, char** argv, shared_ptr<Object> &modelObj) {
        char * modelName;

        // load model
        if (argc<1) {
            cout << "No model given. Specify model name in cmd-line arg." << endl;
            exit(0);
        } else {
            modelName = argv[1];
            cout << "Loading " << modelName << "..." << endl;
        }

        // create a model object with vs and fs shaders
        modelObj = make_shared<Object>("vs.txt", "fs.txt");
        modelObj->loadModel(modelName);
        modelObjPtr = modelObj;
    }

    void createLightCube(shared_ptr<LightCubeObject> &lightCubeObj) {
        // create lightcube object at (15,15,15) with color (1,1,1)
        lightCubeObj = make_shared<LightCubeObject>(&PredefinedModels::lightCubeVertices, 
                                                    cy::Vec3f(15.0, 15.0, 15.0), cy::Vec3f(1.0f,1.0f,1.0f), 
                                                    "lightcube_vs.txt", "lightcube_fs.txt");
        lightCubeObjPtr = lightCubeObj;
    }

    void specialKeyboard(int key, int x, int y) {
        switch (key) {
            case GLUT_KEY_F6:
                // Reload shaders when F6 key is pressed
                modelObjPtr->prog.BuildFiles("vs.txt", "fs.txt");
                modelObjPtr->prog.Bind();
                cout << "Shaders recompiled successfully." << endl;
                glutPostRedisplay();
                break;
            case GLUT_KEY_CTRL_L:
            case GLUT_KEY_CTRL_R:
                controlKeyPressed = true;
                break;
        }
    }

    void keyboard(unsigned char key, int x, int y) {
        if (key == 27) {  // ASCII value for the Esc key
            glutLeaveMainLoop();
        } else {
            cameraPtr->processKeyboard(key, x, y);
        }
        glutPostRedisplay();
    }


    void specialKeyboardUp(int key, int x, int y) {
        switch (key) {
            case GLUT_KEY_CTRL_L:
            case GLUT_KEY_CTRL_R:
                controlKeyPressed = false;
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

        if (controlKeyPressed) {
            lightCubeObjPtr->processMouseMovement(xoffset, yoffset, leftButtonPressed);
        } else {
            cameraPtr->processMouseMovement(xoffset, yoffset, leftButtonPressed);
        }

        glutPostRedisplay();
    }

    void setCallbacks(void (*displayFunc)(void)) {
        // Set up callbacks
        glutDisplayFunc(displayFunc);
        glutKeyboardFunc(keyboard);
        glutSpecialUpFunc(specialKeyboardUp);
        glutMouseFunc(handleMouse);
        glutMotionFunc(mouseMotion);
        glutSpecialFunc(specialKeyboard);
    }

    void initCamera(Camera * camera) {
        cameraPtr = camera;
         // initialize camera
        cameraPtr->setOrthographicMatrix(0.1f, 1500.0f, 500.0f);
        cameraPtr->setPerspectiveMatrix(65,800.0f/600.0f, 2.0f, 1000.0f);
    }


}