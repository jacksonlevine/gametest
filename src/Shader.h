//
// Created by jack on 10/13/2024.
//

#ifndef SHADER_H
#define SHADER_H


#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

namespace jl {

class Shader {
public:
    GLuint shaderID;
    std::string vertexSource;
    std::string fragmentSource;
    Shader(const char *vertex, const char *frag, const char *name);
};

}




#endif //SHADER_H
