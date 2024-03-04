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
shared_ptr<Object> modelObj;
shared_ptr<LightCubeObject> lightCubeObj;
shared_ptr<PlaneObject> planeObj;
cy::GLRenderTexture2D renderBuffer;

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw actual teapot
    modelObj->prog.Bind();
    modelObj->prog["model"] = modelObj->modelMatrix;
    modelObj->prog["view"] = view;
    modelObj->prog["projection"] = proj;
    modelObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    modelObj->prog["lightPos"] = lightCubeObj->worldSpacePos;
    glBindVertexArray(modelObj->VAO);
    glDrawElements(GL_TRIANGLES, modelObj->mesh.NF() * 3, GL_UNSIGNED_INT, 0);

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
    modelObj->prog["lightPos"] = lightCubeObj->worldSpacePos;
    glBindVertexArray(planeObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);  

    glutSwapBuffers();
}


int main(int argc, char** argv) {

    Init::initGL("Project 7: Shadow Mapping", argc, argv);
    Init::setCallbacks(display);

    Init::initUntexturedModel(argc, argv, modelObj, "reflection_vs.txt", "reflection_fs.txt");
    Init::initLightCube(lightCubeObj, "lightcube_vs.txt", "lightcube_fs.txt");


    // initialize some uniforms
    // the teapot model is top down so let's rotate it 90 degrees
    modelObj->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(-90)) * modelObj->modelMatrix;
    modelObj->prog["materialColor"] = cy::Vec3f(1.0f,0.5f,0.5f);
    modelObj->prog["ambientStr"] = 0.1f;
    modelObj->prog["diffuseStr"] = 0.2f;
    modelObj->prog["specularStr"] = 0.8f;
    modelObj->prog["materialShininess"] = modelObj->mesh.M(0).Ns;
    modelObj->prog["lightColor"] = cy::Vec3f(1.0f,1.0f,1.0f);


    Init::initCamera(&camera);

    // set up plane
    planeObj = make_shared<PlaneObject>(&PredefinedModels::planeVertices, cy::Vec3f(0,0,0), "plane_vs.txt", "plane_fs.txt");
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
    planeObj->prog["ambientStr"] = 0.1f;
    planeObj->prog["diffuseStr"] = 0.2f;
    planeObj->prog["specularStr"] = 0.8f;
    planeObj->prog["materialShininess"] = modelObj->mesh.M(0).Ns;
    planeObj->prog["lightColor"] = cy::Vec3f(1.0f,1.0f,1.0f);




    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}