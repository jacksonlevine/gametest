//
// Created by jack on 10/13/2024.
//

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <tiny_gltf.h>
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace jl {

typedef struct
{
    GLuint vbo;
    GLuint texvbo;
    GLuint vao;
    GLuint ebo;
    uint32_t indexcount;
    GLenum drawmode;
    GLenum indextype;
}ModelGLObjects;

typedef struct
{
    std::vector<ModelGLObjects> modelGLObjects;
    std::vector<GLuint> texids;
}ModelAndTextures;

class ModelLoader {
public:
    static ModelAndTextures loadModel(const char* path);
};

} // jl

#endif //MODELLOADER_H
