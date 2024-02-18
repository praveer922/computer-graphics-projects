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

void display() { 
    cy::Matrix4f view = camera.getLookAtMatrix();
    cy::Matrix4f proj = camera.getProjectionMatrix();

    // Your rendering code goes here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // set uniforms    
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
    glutSwapBuffers();
}

void processInputModel(int argc, char** argv) {
    char * modelName;

    // load model
    if (argc<1) {
        cout << "No model given. Specify model name in cmd-line arg." << endl;
        exit(0);
    } else {
        modelName = argv[1];
        cout << "Loading " << modelName << "..." << endl;
    }

    modelObj->loadModel(modelName);
}



int main(int argc, char** argv) {

    Init::initGL("Project4", argc, argv);
    Init::setCallbacks(display);

    // create models/objects
    Init::processInputModel(argc, argv, modelObj);
    // the teapot model is top down so let's rotate it 90 degrees
    modelObj->modelMatrix = cy::Matrix4f::RotationX(Util::degreesToRadians(-90)) * modelObj->modelMatrix;
    Init::createLightCube(lightCubeObj);

    Init::initCamera(&camera);

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


    // Enter the GLUT event loop
    glutMainLoop();

    return 0;
}