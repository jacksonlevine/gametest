//
// Created by jack on 10/13/2024.
//

#include "ModelLoader.h"
#include <cassert>


namespace jl {

GLenum getGLTypeFromTinyGLTFComponentType(int componentType) {
    switch (componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE: return GL_BYTE;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
    case TINYGLTF_COMPONENT_TYPE_SHORT: return GL_SHORT;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return GL_UNSIGNED_SHORT;
    case TINYGLTF_COMPONENT_TYPE_INT: return GL_INT;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return GL_UNSIGNED_INT;
    case TINYGLTF_COMPONENT_TYPE_FLOAT: return GL_FLOAT;
    default: return GL_FLOAT;
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
            GLuint vao, vbo, ebo, boneidvbo, weightvbo, texvbo;

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &texvbo);
            glGenBuffers(1, &boneidvbo);
            glGenBuffers(1, &weightvbo);
            glGenBuffers(1, &ebo);

            glBindVertexArray(vao);
            //Indent operations on this vertex array object
                //Get vertex positions buffer from gltf
                const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
                const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];
                glBindBuffer(positionBufferView.target, vbo);
                glBufferData(positionBufferView.target, positionBufferView.byteLength,
                         &positionBuffer.data.at(0) + positionBufferView.byteOffset,
                         GL_STATIC_DRAW);
                const GLenum postype = getGLTypeFromTinyGLTFComponentType(positionAccessor.componentType);
                glVertexAttribPointer(0, 3, postype, GL_FALSE, positionAccessor.ByteStride(positionBufferView), nullptr);
                glEnableVertexAttribArray(0);



                //Get texture UVs buffer from gltf
                const tinygltf::Accessor& texCoordAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
                const tinygltf::Buffer& texCoordBuffer = model.buffers[texCoordBufferView.buffer];
                glBindBuffer(texCoordBufferView.target, texvbo);
                glBufferData(texCoordBufferView.target, texCoordBufferView.byteLength,
                         &texCoordBuffer.data.at(0) + texCoordBufferView.byteOffset,
                         GL_STATIC_DRAW);
                const GLenum textype = getGLTypeFromTinyGLTFComponentType(texCoordAccessor.componentType);
                glVertexAttribPointer(1, 2, textype, GL_FALSE, texCoordAccessor.ByteStride(texCoordBufferView), nullptr);
                glEnableVertexAttribArray(1);

                //Get boneids aka joints buffer
                const tinygltf::Accessor& boneIDAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
                const tinygltf::BufferView& boneIDBufferView = model.bufferViews[boneIDAccessor.bufferView];
                const tinygltf::Buffer& boneIDBuffer = model.buffers[boneIDBufferView.buffer];
                glBindBuffer(boneIDBufferView.target, boneidvbo);
                glBufferData(boneIDBufferView.target, boneIDBufferView.byteLength,
                         &boneIDBuffer.data.at(0) + boneIDBufferView.byteOffset,
                         GL_STATIC_DRAW);
                const GLenum boneidtype = getGLTypeFromTinyGLTFComponentType(boneIDAccessor.componentType);
                glVertexAttribPointer(2, 4, boneidtype, GL_FALSE, boneIDAccessor.ByteStride(boneIDBufferView), nullptr);
                glEnableVertexAttribArray(2);


                //Get indices buffer from gltf
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];
                glBindBuffer(indexBufferView.target, ebo);
                glBufferData(indexBufferView.target, indexBufferView.byteLength,
                         &indexBuffer.data.at(0) + indexBufferView.byteOffset,
                         GL_STATIC_DRAW);
                //Leave the vao with the index buffer bound, ready to go for drawElements
            glBindVertexArray(0);

            const GLenum indextype = getGLTypeFromTinyGLTFComponentType(indexAccessor.componentType); //We need this when we draw it too for some reason, so holding on to it

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

        GLenum pixtype = getGLTypeFromTinyGLTFComponentType(image.pixel_type);

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
            0,                // border (this must be 0 according to gl docs lol)
            format,           // format of the pixel data
            pixtype,          // type of the pixel data
            &image.image[0]   // pointer to the actual pixel data
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        texids.push_back(texid);
    }

    return ModelAndTextures{modelGLObjects, texids};
}

} // jl