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
PlaneObject* planeObj;
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
    Init::processInputModel(argc, argv, modelObj);
    // the teapot model is top down so let's rotate it 90 degrees
    modelObj->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(-90)) * modelObj->modelMatrix;
    Init::createLightCube(lightCubeObj);

    Init::initCamera(&camera);
    Init::initPlaneCamera(&planeCamera);

    // set up VAO and VBO and EBO and NBO
    glGenVertexArrays(1, &(modelObj->VAO)); 
    glBindVertexArray(modelObj->VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f)*modelObj->positions.size(), modelObj->positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * modelObj->mesh.NF() * 3, &modelObj->mesh.F(0), GL_STATIC_DRAW);

    GLuint normalVBO;
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * modelObj->normals.size(), modelObj->normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1); // Assuming attribute index 1 for normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Create and bind VBO for texture coordinates
    GLuint texCoordVBO;
    glGenBuffers(1, &texCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * modelObj->texCoords.size(), modelObj->texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2); // Assuming attribute index 2 for texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(cy::Vec3f), (void*)0);

    // teapot texture
    GLuint teapotTexture;
    glGenTextures(1, &teapotTexture);  
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, teapotTexture);  
    modelObj->prog["ambientMap"] = 0;
    modelObj->prog["diffuseMap"] = 0; 
    modelObj->prog["materialAmbient"] = cy::Vec3f(modelObj->mesh.M(0).Ka[0], modelObj->mesh.M(0).Ka[1], modelObj->mesh.M(0).Ka[2]);
    modelObj->prog["materialDiffuse"] = cy::Vec3f(modelObj->mesh.M(0).Kd[0], modelObj->mesh.M(0).Kd[1], modelObj->mesh.M(0).Kd[2]);
    modelObj->prog["materialSpecular"] = cy::Vec3f(modelObj->mesh.M(0).Ks[0], modelObj->mesh.M(0).Ks[1], modelObj->mesh.M(0).Ks[2]);
    modelObj->prog["materialShininess"] = modelObj->mesh.M(0).Ns;

    std::vector<unsigned char> image; // The raw pixels
    unsigned width, height;

    // Decode the image
    unsigned error = lodepng::decode(image, width, height, modelObj->mesh.M(0).map_Kd.data);
    if (error) {
        std::cerr << "Error loading texture: " << lodepng_error_text(error) << std::endl;
        return 0;
    } else {
        std::cout << "Diffuse texture map loaded successfully." << std::endl;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    // load specular texture
    GLuint specularTexture;
    glGenTextures(1, &specularTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularTexture);
    modelObj->prog["specularMap"] = 1; // set to texture 1

    std::vector<unsigned char> specular_image; // The raw pixels
    unsigned spec_w, spec_h;

    // Decode the image
    unsigned error_spec = lodepng::decode(specular_image, spec_w, spec_h, modelObj->mesh.M(0).map_Ks.data);
    if (error) {
        std::cerr << "Error loading texture: " << lodepng_error_text(error_spec) << std::endl;
        return 0;
    } else {
        std::cout << "Specular texture map loaded successfully." << std::endl;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spec_w, spec_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &specular_image[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set up light cube

    glGenVertexArrays(1, &(lightCubeObj->VAO)); 
    glBindVertexArray(lightCubeObj->VAO);

    GLuint lightCubeVBO;
    glGenBuffers(1, &lightCubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * lightCubeObj->vertices->size(), lightCubeObj->vertices->data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


    // set up framebuffer for rendering to plane
    glActiveTexture(GL_TEXTURE2);
    renderBuffer.Initialize(true, 3, 800, 600);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Mip-mapping 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Bilinear filtering for magnification
    GLfloat maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

    // set up plane itself
    planeObj = new PlaneObject(&PredefinedModels::planeVertices, cy::Vec3f(0,0,0), "plane_vs.txt", "plane_fs.txt");
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