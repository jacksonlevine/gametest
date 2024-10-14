//
// Created by jack on 10/13/2024.
//

#include "ModelLoader.h"

namespace jl {

GLenum getGLType(int componentType) {
    switch (componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE: return GL_BYTE;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
    case TINYGLTF_COMPONENT_TYPE_SHORT: return GL_SHORT;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
    case TINYGLTF_COMPONENT_TYPE_INT: return GL_INT;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return GL_UNSIGNED_INT;
    case TINYGLTF_COMPONENT_TYPE_FLOAT: return GL_FLOAT;
    default: return GL_FLOAT; // Default fallback
    }
}

ModelAndTextures ModelLoader::loadModel(const char* path)
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

    std::vector<ModelGLObjects> modelGLObjects;
    std::vector<GLuint> texids;

    for (const tinygltf::Mesh& mesh : model.meshes) {
        for (const tinygltf::Primitive& primitive : mesh.primitives) {


            const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

            const unsigned char* indexBufferData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

            // Get vertex position data
            const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
            const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
            const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];
            const auto* positionBufferData = reinterpret_cast<const float*>(&positionBuffer.data[positionBufferView.byteOffset + positionAccessor.byteOffset]);

            // Optionally, get texture coordinate data
            const tinygltf::Accessor& texCoordAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
            const tinygltf::BufferView& texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
            const tinygltf::Buffer& texCoordBuffer = model.buffers[texCoordBufferView.buffer];
            const auto* texCoordBufferData = reinterpret_cast<const float*>(&texCoordBuffer.data[texCoordBufferView.byteOffset + texCoordAccessor.byteOffset]);


            // Determine stride for position data
            size_t posStride = positionAccessor.ByteStride(positionBufferView);
            if (posStride == 0) posStride = sizeof(float) * 3; // Default stride if not specified

            // Determine stride for texture coordinate data
            size_t texStride = texCoordAccessor.ByteStride(texCoordBufferView);
            if (texStride == 0) texStride = sizeof(float) * 2; // Default stride if not specified



            GLuint vao, vbo, ebo, texvbo;
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &texvbo);
            glGenBuffers(1, &ebo);
            std::cout << "Genned vao: " << std::to_string(vao) << "\n";

            glBindVertexArray(vao);

            GLenum postype = getGLType(positionAccessor.componentType);
            // Upload vertex position data
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, positionBufferView.byteLength, positionBufferData, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, postype, GL_FALSE, posStride, (void*)0);  // Assume 3 floats per position
            glEnableVertexAttribArray(0);

            GLenum textype = getGLType(positionAccessor.componentType);
            // Upload texture coordinate data
            glBindBuffer(GL_ARRAY_BUFFER, texvbo);
            glBufferData(GL_ARRAY_BUFFER, texCoordBufferView.byteLength, texCoordBufferData, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 2, textype, GL_FALSE, texStride, (void*)0);  // Assume 2 floats per texture coordinate
            glEnableVertexAttribArray(1);

            // Upload index data
            GLenum indextype = getGLType(indexAccessor.componentType);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferView.byteLength, indexBufferData, GL_STATIC_DRAW);

            glBindVertexArray(0);

            modelGLObjects.emplace_back(ModelGLObjects{vbo,
                texvbo,
                vao,
                ebo,
                static_cast<uint32_t>(indexAccessor.count),
                static_cast<GLenum>(primitive.mode),
                indextype
            });
        }
    }

    for (const auto& texture : model.textures) {

        const tinygltf::Image& image = model.images[texture.source];

        GLuint texid;
        glGenTextures(1, &texid);

        glBindTexture(GL_TEXTURE_2D, texid);

        GLenum format = GL_RGB;
        if (image.component == 1) {
            format = GL_RED;
        } else if (image.component == 3) {
            format = GL_RGB;
        } else if (image.component == 4) {
            format = GL_RGBA;
        }

        glTexImage2D(
            GL_TEXTURE_2D,
            0,                // mipmap level
            format,           // internal format
            image.width,      // width
            image.height,     // height
            0,                // border (must be 0)
            format,           // format of the pixel data
            GL_UNSIGNED_BYTE, // type of the pixel data
            &image.image[0]   // pointer to the actual pixel data
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texids.push_back(texid);
    }

    return ModelAndTextures{modelGLObjects, texids};
}

} // jl