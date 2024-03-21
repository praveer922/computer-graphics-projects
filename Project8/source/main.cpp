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
shared_ptr<LightCubeObject> lightCubeObj;
shared_ptr<PlaneObject> planeObj;
bool isSpaceKeyPressed = false;
float tessLevel = 16.0f;
GLuint depthMapFBO;
GLuint depthMapTexture;
shared_ptr<PlaneObject> depthScreen;
float dispFactor = 20.0f;

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

     // render to depth map
    glCullFace(GL_FRONT);
    // first update light camera 
    lightCamera.setPosition(lightCubeObj->worldSpacePos);
    lightCamera.setFrontDirection(-(lightCubeObj->worldSpacePos));
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    // draw the plane
    planeObj->depthMapProg.Bind();
    planeObj->depthMapProg["model"] = planeObj->modelMatrix;
    planeObj->depthMapProg["lightSpaceMatrix"] = lightCamera.getProjectionMatrix() * lightCamera.getLookAtMatrix();
    planeObj->depthMapProg["tessLevel"] = 20.0f;
    planeObj->depthMapProg["dispFactor"] = dispFactor;
    glBindVertexArray(planeObj->VAO);
    glDrawArrays(GL_PATCHES, 0, 6);  

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCullFace(GL_BACK);
    
    // draw actual scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    planeObj->prog["tessLevel"] = tessLevel;
    planeObj->prog["lightSpaceMatrix"] = lightCamera.getProjectionMatrix() * lightCamera.getLookAtMatrix();
    planeObj->prog["dispFactor"] = dispFactor;
    glBindVertexArray(planeObj->VAO);
    glDrawArrays(GL_PATCHES, 0, 6);  


    // draw the plane with triangulation (geometry shader)
    if (isSpaceKeyPressed) {
        planeObj->geometryProg.Bind();
        planeObj->geometryProg["model"] = planeObj->modelMatrix;
        planeObj->geometryProg["view"] = view;
        planeObj->geometryProg["projection"] = proj;
        planeObj->geometryProg["tessLevel"] = tessLevel;
        planeObj->geometryProg["dispFactor"] = dispFactor;
        glDrawArrays(GL_PATCHES, 0, 6);  
    }

    // draw depth map debug screen

    depthScreen->prog.Bind();
    depthScreen->prog["model"] = depthScreen->modelMatrix;
    depthScreen->prog["view"] = view;
    depthScreen->prog["projection"] = proj;
    depthScreen->prog["cameraWorldSpacePos"] = camera.getPosition();
    depthScreen->prog["lightPos"] = lightCubeObj->worldSpacePos;
    glBindVertexArray(planeObj->VAO);
    //glDrawArrays(GL_TRIANGLES, 0, 6); 


    glutSwapBuffers();
}


int main(int argc, char** argv) {

    Init::initGL("Project 8: Tesselation", argc, argv);
    Init::setCallbacks(display);

    Init::initLightCube(lightCubeObj, "lightcube_vs.txt", "lightcube_fs.txt");
    
    // init cameras
    Init::initCamera(&camera);
    lightCamera.setPerspectiveMatrix(65,800.0f/600.0f, 2.0f, 100.0f);

    // set up plane
    planeObj = make_shared<PlaneObject>(&PredefinedModels::planeVertices, cy::Vec3f(0,0,0), "plane_vs.txt", "plane_fs.txt");
    planeObj->prog.BuildFiles("plane_vs.txt", "plane_fs.txt", nullptr, "ts_control.txt", "ts_evaluation.txt");
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
    GLfloat maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture filtering parameters (use mipmaps for minification)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Enable anisotropic filtering if supported
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);

    planeObj->prog["normalMapTexture"] = 0;

    // set up geometry shader program
    planeObj->geometryProg.BuildFiles("triangulation_vs.txt", "triangulation_fs.txt", "gs.txt", "ts_control.txt", "ts_evaluation.txt");
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // load displacement map
    std::vector<unsigned char> disp_image; 

    // Decode the image
    if (argv[2] != nullptr) {
        error = lodepng::decode(disp_image, width, height,argv[2]);
        if (error) {
            std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
            return 0;
        } else {
            std::cout << "Displacement map loaded successfully." << std::endl;
        }
    } else {
        dispFactor = 0.0f;
    }
    

    GLuint dispMapTexture;
    glGenTextures(1, &dispMapTexture);  
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, dispMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &disp_image[0]);
    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture filtering parameters (use mipmaps for minification)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Enable anisotropic filtering if supported
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);

    planeObj->prog["dispMap"] = 1;
    planeObj->geometryProg["dispMap"] = 1;

    //create depth map
    glGenFramebuffers(1, &depthMapFBO);  
    glGenTextures(1, &depthMapTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 
                800, 600, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    planeObj->depthMapProg.BuildFiles("depthmap_vs.txt", "depthmap_fs.txt", nullptr, "ts_control.txt", "ts_evaluation.txt");
    planeObj->depthMapProg["dispMap"] = 1;
    planeObj->prog["shadowMap"] = 2;

    depthScreen = make_shared<PlaneObject>(&PredefinedModels::planeVertices, cy::Vec3f(0,0,0), "depthscreen_vs.txt", "depthscreen_fs.txt");
    depthScreen->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(90)) * depthScreen->modelMatrix;
    depthScreen->modelMatrix = cy::Matrix4f::Scale(5.5) * depthScreen->modelMatrix;
    depthScreen->prog["depthMap"] = 2;

    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}