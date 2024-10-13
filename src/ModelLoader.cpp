//
// Created by jack on 10/13/2024.
//

#include "ModelLoader.h"

namespace jl {
    tinygltf::Model ModelLoader::loadModel(const char* path)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        //const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path); // for .gltf
        const bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path); // for .glb

        if (!warn.empty()) {
            std::cout << "Warn: " << warn.c_str() << "\n";
        }

        if (!err.empty()) {
            std::cout << "Err: " << err.c_str() << "\n";
        }

        if (!ret) {
            std::cout << "Failed to parse glTF at " << path << "\n";
        }

        return model;
    }
} // jl