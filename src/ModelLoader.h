//
// Created by jack on 10/13/2024.
//

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <tiny_gltf.h>
#include <iostream>
#include <string>

namespace jl {

class ModelLoader {
public:
static tinygltf::Model loadModel(const char* path);
};

} // jl

#endif //MODELLOADER_H
