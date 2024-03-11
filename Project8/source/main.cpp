#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "includes/cyTriMesh.h"
#include "includes/cyMatrix.h"
#include "includes/cyGL.h"
#include "includes/lodepng.h"
#include "Camera.h"
#include "Object.h"
#include "Init.h"
#include <memory>
#include <iostream>

using namespace std;

Camera camera;
shared_ptr<LightCubeObject> lightCubeObj;
shared_ptr<PlaneObject> planeObj;
bool isSpaceKeyPressed = false;

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    // draw actual scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw the light cube
    lightCubeObj->prog.Bind();
    lightCubeObj->prog["model"] = lightCubeObj->modelMatrix;
    lightCubeObj->prog["view"] = view;
    lightCubeObj->prog["projection"] = proj;
    glBindVertexArray(lightCubeObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // draw the plane
    planeObj->prog.Bind();
    planeObj->prog["model"] = planeObj->modelMatrix;
    planeObj->prog["view"] = view;
    planeObj->prog["projection"] = proj;
    planeObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    planeObj->prog["lightPos"] = lightCubeObj->worldSpacePos;
    glBindVertexArray(planeObj->VAO);
    glDrawArrays(GL_PATCHES, 0, 6);  


    // draw the plane with triangulation (geometry shader)
    if (isSpaceKeyPressed) {
        planeObj->geometryProg.Bind();
        planeObj->geometryProg["model"] = planeObj->modelMatrix;
        planeObj->geometryProg["view"] = view;
        planeObj->geometryProg["projection"] = proj;
        glDrawArrays(GL_PATCHES, 0, 6);  
    }


    glutSwapBuffers();
}


int main(int argc, char** argv) {

    Init::initGL("Project 8: Tesselation", argc, argv);
    Init::setCallbacks(display);

    Init::initLightCube(lightCubeObj, "lightcube_vs.txt", "lightcube_fs.txt");
    
    // init cameras
    Init::initCamera(&camera);

    // set up plane
    planeObj = make_shared<PlaneObject>(&PredefinedModels::planeVertices, cy::Vec3f(0,0,0), "plane_vs.txt", "plane_fs.txt");
    planeObj->prog.BuildFiles("plane_vs.txt", "plane_fs.txt", nullptr, "ts_control.txt", "ts_evaluation.txt");
    planeObj->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(60)) * planeObj->modelMatrix;
    // also scale it to be bigger
    planeObj->modelMatrix = cy::Matrix4f::Scale(5.5) * planeObj->modelMatrix;

    glGenVertexArrays(1, &(planeObj->VAO)); 
    glBindVertexArray(planeObj->VAO);

    GLuint planeVBO;
    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planeObj->vertices->size(), planeObj->vertices->data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    planeObj->prog["materialColor"] = cy::Vec3f(1.0f,1.0f,1.0f);
    planeObj->prog["ambientStr"] = 0.0f;
    planeObj->prog["diffuseStr"] = 0.3f;
    planeObj->prog["specularStr"] = 0.8f;
    planeObj->prog["materialShininess"] = 32.0f;
    planeObj->prog["lightColor"] = cy::Vec3f(1.0f,1.0f,1.0f);

    // set up normal texture
    std::vector<unsigned char> image; // The raw pixels
    unsigned width, height;

    // Decode the image
    unsigned error = lodepng::decode(image, width, height,argv[1]);
    if (error) {
        std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
        return 0;
    } else {
        std::cout << "Normal texture map loaded successfully." << std::endl;
    }

    GLuint normalMapTexture;
    glGenTextures(1, &normalMapTexture);  
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, normalMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    planeObj->prog["normalMapTexture"] = 0;

    // set up geometry shader program
    planeObj->geometryProg.BuildFiles("triangulation_vs.txt", "triangulation_fs.txt", "gs.txt", "ts_control.txt", "ts_evaluation.txt");
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}