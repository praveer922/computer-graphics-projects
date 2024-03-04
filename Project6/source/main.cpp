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
shared_ptr<PlaneObject> planeObj;
cy::GLRenderTexture2D renderBuffer;

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw  inverted cubemap
    glDepthMask(GL_FALSE);
    cubeMapObj->prog.Bind();
    cubeMapObj->prog["model"] = cy::Matrix4f::Translation(camera.getPosition());
    cubeMapObj->prog["view"] = view;
    cubeMapObj->prog["projection"] = proj;
    glBindVertexArray(cubeMapObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    // render inverted teapot to framebuffer
    renderBuffer.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    modelObj->prog.Bind();
    modelObj->prog["model"] = cy::Matrix4f::Translation(cy::Vec3f(0.0f,-16.0f,0.0f)) * cy::Matrix4f::Scale(1.0,-1.0,1.0,1.0) * modelObj->modelMatrix;
    modelObj->prog["view"] = view;
    modelObj->prog["projection"] = proj;
    modelObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    modelObj->prog["isReflection"] = true;
    glBindVertexArray(modelObj->VAO);
    glDrawElements(GL_TRIANGLES, modelObj->mesh.NF() * 3, GL_UNSIGNED_INT, 0);
    renderBuffer.Unbind();
    renderBuffer.BuildTextureMipmaps();

    // draw actual teapot
    modelObj->prog.Bind();
    modelObj->prog["model"] = modelObj->modelMatrix;
    modelObj->prog["view"] = view;
    modelObj->prog["projection"] = proj;
    modelObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    modelObj->prog["isReflection"] = false;
    glBindVertexArray(modelObj->VAO);
    glDrawElements(GL_TRIANGLES, modelObj->mesh.NF() * 3, GL_UNSIGNED_INT, 0);

    // draw the plane
    planeObj->prog.Bind();
    planeObj->prog["model"] = planeObj->modelMatrix;
    planeObj->prog["view"] = camera.getLookAtMatrix();
    planeObj->prog["projection"] = camera.getProjectionMatrix();
    planeObj->prog["cameraWorldSpacePos"] = camera.getPosition();
    glBindVertexArray(planeObj->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);  

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
    
    planeObj->prog["renderedTexture"] = 1;
    planeObj->prog["skybox"] = 0;
    planeObj->prog["envLightIntensity"] = 0.75f;

     // set up render buffer  that we will render the reflection of the teapot to
    glActiveTexture(GL_TEXTURE1);
    renderBuffer.Initialize(true, 4, 800, 600);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mip-mapping 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear filtering for magnification
    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);



    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}