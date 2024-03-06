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
Camera lightCamera;
shared_ptr<Object> modelObj;
shared_ptr<LightCubeObject> lightCubeObj;
shared_ptr<PlaneObject> planeObj;
shared_ptr<PlaneObject> depthScreen;
GLuint depthMapFBO;
GLuint depthMapTexture;

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    // render to depth map
    // first update light camera 
    lightCamera.setPosition(lightCubeObj->worldSpacePos);
    lightCamera.setFrontDirection(-(lightCubeObj->worldSpacePos));
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    // draw  teapot
    modelObj->depthMapProg.Bind();
    modelObj->depthMapProg["model"] = modelObj->modelMatrix;
    modelObj->depthMapProg["lightSpaceMatrix"] = lightCamera.getProjectionMatrix() * lightCamera.getLookAtMatrix();
    glBindVertexArray(modelObj->VAO);
    glDrawElements(GL_TRIANGLES, modelObj->mesh.NF() * 3, GL_UNSIGNED_INT, 0);

    // draw the plane
    planeObj->depthMapProg.Bind();
    planeObj->depthMapProg["model"] = planeObj->modelMatrix;
    planeObj->depthMapProg["lightSpaceMatrix"] = lightCamera.getProjectionMatrix() * lightCamera.getLookAtMatrix();
    glBindVertexArray(planeObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);  

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // draw actual scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    // draw actual teapot
    modelObj->prog.Bind();
    modelObj->prog["model"] = modelObj->modelMatrix;
    modelObj->prog["view"] = view;
    modelObj->prog["projection"] = proj;
    modelObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    modelObj->prog["lightPos"] = lightCubeObj->worldSpacePos;
    glBindVertexArray(modelObj->VAO);
    //glDrawElements(GL_TRIANGLES, modelObj->mesh.NF() * 3, GL_UNSIGNED_INT, 0);

    // draw the light cube
    lightCubeObj->prog.Bind();
    lightCubeObj->prog["model"] = lightCubeObj->modelMatrix;
    lightCubeObj->prog["view"] = view;
    lightCubeObj->prog["projection"] = proj;
    glBindVertexArray(lightCubeObj->VAO);
    //glDrawArrays(GL_TRIANGLES, 0, 36);

    // draw the plane
    planeObj->prog.Bind();
    planeObj->prog["model"] = planeObj->modelMatrix;
    planeObj->prog["view"] = view;
    planeObj->prog["projection"] = proj;
    planeObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    planeObj->prog["lightPos"] = lightCubeObj->worldSpacePos;
    glBindVertexArray(planeObj->VAO);
    //glDrawArrays(GL_TRIANGLES, 0, 6);  


    // draw depth map debug screen

    depthScreen->prog.Bind();
    depthScreen->prog["model"] = depthScreen->modelMatrix;
    depthScreen->prog["view"] = view;
    depthScreen->prog["projection"] = proj;
    depthScreen->prog["cameraWorldSpacePos"] = camera.getPosition();
    depthScreen->prog["lightPos"] = lightCubeObj->worldSpacePos;
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

    // init cameras
    Init::initCamera(&camera);
    lightCamera.setPerspectiveMatrix(65,800.0f/600.0f, 2.0f, 100.0f);

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


    //create depth map
    glGenFramebuffers(1, &depthMapFBO);  
    glGenTextures(1, &depthMapTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
                800, 600, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    // create another plane for debugging depth map
    depthScreen = make_shared<PlaneObject>(&PredefinedModels::planeVertices, cy::Vec3f(0,0,0), "depthscreen_vs.txt", "depthscreen_fs.txt");
    depthScreen->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(90)) * depthScreen->modelMatrix;
    depthScreen->modelMatrix = cy::Matrix4f::Scale(5.5) * depthScreen->modelMatrix;
    depthScreen->prog["depthMap"] = 0;

    // set up depth map shader programs for all objects
    modelObj->depthMapProg.BuildFiles("depthmap_vs.txt", "depthmap_fs.txt");
    planeObj->depthMapProg.BuildFiles("depthmap_vs.txt", "depthmap_fs.txt");

    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}