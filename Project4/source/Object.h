#pragma once
#include <string>
#include <iostream>
#include <GL/freeglut.h>
#include "includes/cyTriMesh.h"
#include "includes/cyMatrix.h"
#include "Util.h"


using namespace std;

class Object {
public:
    Object(const std::string& vsFile, const std::string& fsFile) 
        : modelMatrix(cy::Matrix4f(1.0)), vsFile(vsFile), fsFile(fsFile) {
            prog.BuildFiles(vsFile.c_str(),fsFile.c_str());
    }

    cy::Matrix4f modelMatrix; // Model matrix for the object
    string vsFile;    // Vertex shader file path
    string fsFile;    // Fragment shader file path
    cy::GLSLProgram prog;
    GLuint VAO;
    
    cy::TriMesh mesh;

    void loadModel(char * filename) {
        bool success = mesh.LoadFromFileObj(filename);
        if (!success) {
            cout << "Model loading failed." << endl;
            exit(0);
        } else {
            cout << "Loaded model successfully." << endl;
            mesh.ComputeNormals();
            mesh.ComputeBoundingBox();
        }

        // automatically center the model in world space
        cy::Vec3f center = (mesh.GetBoundMin() + mesh.GetBoundMax()) * 0.5f;
        modelMatrix *= cy::Matrix4f::Translation(-center); 
    }
    
};


class CubeObject : public Object {
public:
    CubeObject(std::vector<float>* vertices, cy::Vec3f worldSpacePos, const std::string& vsFile, const std::string& fsFile) 
        : Object(vsFile, fsFile), vertices(vertices), worldSpacePos(worldSpacePos), rot_y(0.0f), rot_z(0.0f) {
            prog.BuildFiles(vsFile.c_str(),fsFile.c_str());
            // update the model matrix so that the vertices are translated to the given worldSpacePos
            modelMatrix = cy::Matrix4f::Translation(worldSpacePos);
    }
    
    std::vector<float>* vertices; // Pointer to a vector of vertices
    cy::Vec3f worldSpacePos;
    float rot_y;
    float rot_z;

    void updateModelMatrix() {
        // update model matrix based on worldSpacePos()
        modelMatrix = cy::Matrix4f::Translation(worldSpacePos);
    }


    void processMouseMovement(float xoffset, float yoffset, bool leftButtonPressed) {
        if (leftButtonPressed) {
            rot_y -=xoffset * 0.01f;
            rot_z += yoffset * 0.01f;
        }

        // update worldSpacePos and model matrix with new rot values
        worldSpacePos = cy::Matrix3f::RotationY(Util::degreesToRadians(rot_y)) * cy::Matrix3f::RotationZ(Util::degreesToRadians(rot_z)) * worldSpacePos;
        updateModelMatrix();

    }
    
};