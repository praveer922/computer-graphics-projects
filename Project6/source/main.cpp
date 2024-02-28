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
shared_ptr<LightCubeObject> cubeMapObj;

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw cubemap
    glDepthMask(GL_FALSE);
    cubeMapObj->prog.Bind();
    cubeMapObj->prog["model"] = cy::Matrix4f::Translation(camera.getPosition());
    cubeMapObj->prog["view"] = view;
    cubeMapObj->prog["projection"] = proj;
    glBindVertexArray(cubeMapObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
    // draw model
    modelObj->prog.Bind();
    modelObj->prog["model"] = modelObj->modelMatrix;
    modelObj->prog["view"] = view;
    modelObj->prog["projection"] = proj;
    modelObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    glBindVertexArray(modelObj->VAO);
    glDrawElements(GL_TRIANGLES, modelObj->mesh.NF() * 3, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();
}


int main(int argc, char** argv) {

    Init::initGL("Project6: Environment Mapping", argc, argv);
    Init::setCallbacks(display);

    Init::initUntexturedModel(argc, argv, modelObj, "reflection_vs.txt", "reflection_fs.txt");
    Init::initLightCube(cubeMapObj, "cubemap_vs.txt", "cubemap_fs.txt");


    // initialize some uniforms
    // the teapot model is top down so let's rotate it 90 degrees
    modelObj->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(-90)) * modelObj->modelMatrix;
    modelObj->prog["skybox"] = 0;
    modelObj->prog["materialColor"] = cy::Vec3f(1.0f,1.0f,1.0f);
    modelObj->prog["ambientStr"] = 0.1f;
    modelObj->prog["diffuseStr"] = 0.2f;
    modelObj->prog["specularStr"] = 0.8f;
    modelObj->prog["materialShininess"] = modelObj->mesh.M(0).Ns;
    modelObj->prog["lightColor"] = cy::Vec3f(1.0f,1.0f,1.0f);
    modelObj->prog["lightPos"] = cy::Vec3f(15.0, -2.0, 10.0);
    modelObj->prog["envLightIntensity"] = 0.75f;
    


    cubeMapObj->prog["skybox"] = 0;



    Init::initCubeMap({"cubemap/cubemap_posx.png", "cubemap/cubemap_negx.png",
    "cubemap/cubemap_posy.png", "cubemap/cubemap_negy.png",
    "cubemap/cubemap_posz.png", "cubemap/cubemap_negz.png", });

    Init::initCamera(&camera);


    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}