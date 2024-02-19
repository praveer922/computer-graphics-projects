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
Camera planeCamera;
shared_ptr<Object> modelObj;
shared_ptr<LightCubeObject> lightCubeObj;
shared_ptr<PlaneObject> planeObj;
cy::GLRenderTexture2D renderBuffer;

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    renderBuffer.Bind();
    // render to framebuffer (first pass)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    modelObj->prog.Bind();
    modelObj->prog["model"] = modelObj->modelMatrix;
    modelObj->prog["view"] = view;
    modelObj->prog["projection"] = proj;
    modelObj->prog["lightWorldSpacePos"] = lightCubeObj->worldSpacePos;
    modelObj->prog["lightColor"] = lightCubeObj->lightColor;
    glBindVertexArray(modelObj->VAO);
    glDrawElements(GL_TRIANGLES, modelObj->mesh.NF() * 3, GL_UNSIGNED_INT, 0);
    lightCubeObj->prog.Bind();
    lightCubeObj->prog["model"] = lightCubeObj->modelMatrix;
    lightCubeObj->prog["view"] = view;
    lightCubeObj->prog["projection"] = proj;
    glBindVertexArray(lightCubeObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    renderBuffer.Unbind();
    renderBuffer.BuildTextureMipmaps();

    // draw the plane (second pass)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    planeObj->prog.Bind();
    planeObj->prog["model"] = planeObj->modelMatrix;
    planeObj->prog["view"] = planeCamera.getLookAtMatrix();
    planeObj->prog["projection"] = planeCamera.getProjectionMatrix();
    glBindVertexArray(planeObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);  
    glutSwapBuffers();
}


int main(int argc, char** argv) {

    Init::initGL("Project5", argc, argv);
    Init::setCallbacks(display);

    // create models/objects
    Init::initModel(argc, argv, modelObj);
    Init::initLightCube(lightCubeObj);

    Init::initCamera(&camera);
    Init::initPlaneCamera(&planeCamera);

    // set up framebuffer for rendering to plane
    glActiveTexture(GL_TEXTURE2);
    renderBuffer.Initialize(true, 3, 800, 600);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mip-mapping 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear filtering for magnification
    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

    // set up plane itself
    planeObj = make_shared<PlaneObject>(&PredefinedModels::planeVertices, cy::Vec3f(0,0,0), "plane_vs.txt", "plane_fs.txt");
    // plane is flat so lets rotate 90 degrees
    planeObj->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(90)) * planeObj->modelMatrix;
    // also scale it to be bigger
    planeObj->modelMatrix = cy::Matrix4f::Scale(2.0) * planeObj->modelMatrix;

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

    planeObj->prog["renderedTexture"] = 2;


    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}