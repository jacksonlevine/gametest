//
// Created by jack on 10/13/2024.
//

#include "ModelLoader.h"
#include <cassert>
#include <glm/gtc/type_ptr.hpp>


namespace jl {
    tinygltf::Model MODEL;

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

int getNumberOfComponents(int acctype)
{
    int size = 1;
    if (acctype == TINYGLTF_TYPE_SCALAR) {
        size = 1;
    } else if (acctype == TINYGLTF_TYPE_VEC2) {
        size = 2;
    } else if (acctype == TINYGLTF_TYPE_VEC3) {
        size = 3;
    } else if (acctype == TINYGLTF_TYPE_VEC4) {
        size = 4;
    } else {
        assert(0);
    }
    return size;
}

GLuint createSSBOForBoneTransformsAndInvBindMats()
{
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(glm::mat4) * 500 * 2,
                 nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    return ssbo;
}

void storeBoneTransformsAndInvBindMats(
    GLuint ssbo, void* boneData, void* invBindData)
{
    //Just 500 mat4's each, we'll hope over-reading doesnt segfault for now
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    0,
                    sizeof(glm::mat4) * JOINT_COUNT,
                    boneData);

    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    sizeof(glm::mat4) * JOINT_COUNT,
                    sizeof(glm::mat4) * JOINT_COUNT,
                    invBindData);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

    void storeBoneTransforms(
    GLuint ssbo, void* boneData)
{
    //Just 500 mat4's each, we'll hope over-reading doesnt segfault for now
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    0,
                    sizeof(glm::mat4) * 500,
                    boneData);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void updateInterpolateBoneTransforms(float anim_time) {
    // Step 1: Access the animation
    assert(MODEL.animations.size() > 0);
    const tinygltf::Animation& animation = MODEL.animations.at(0); // Assuming the first animation is "walk"
    std::vector<glm::mat4> newBoneTransforms(JOINT_COUNT);

    // Step 2: Iterate through each channel in the animation
    for (const auto& channel : animation.channels) {
        // Access the input and output accessors directly from the channel
        const tinygltf::Accessor& inputAccessor = MODEL.accessors[channel.sampler];
        const tinygltf::Accessor& outputAccessor = MODEL.accessors[channel.sampler + 1]; // Assuming this is how the outputs are indexed

        // Get keyframe data
        const float* inputs = reinterpret_cast<const float*>(&MODEL.buffers[MODEL.bufferViews[inputAccessor.bufferView].buffer].data[inputAccessor.byteOffset]);
        const glm::vec4* outputs = reinterpret_cast<const glm::vec4*>(&MODEL.buffers[MODEL.bufferViews[outputAccessor.bufferView].buffer].data[outputAccessor.byteOffset]);

        // Assuming inputs are time values and outputs are transforms (translation, rotation, scale)
        size_t numKeyframes = inputAccessor.count;

        // Step 3: Interpolate for the target joint based on the channel
        int jointNodeIndex = channel.target_node; // Get the joint index from the channel
        const tinygltf::Node& jointNode = MODEL.nodes[jointNodeIndex];

        glm::mat4 jointMatrix = glm::mat4(1.0f); // Initialize the joint matrix

        // Interpolate keyframes for translation, rotation, and scale based on 'anim_time'
        for (size_t k = 0; k < numKeyframes - 1; ++k) {
            if (anim_time >= inputs[k] && anim_time <= inputs[k + 1]) {
                // Linear interpolation
                float t = (anim_time - inputs[k]) / (inputs[k + 1] - inputs[k]);

                glm::vec3 translation = glm::mix(glm::vec3(outputs[k].x, outputs[k].y, outputs[k].z),
                                                  glm::vec3(outputs[k + 1].x, outputs[k + 1].y, outputs[k + 1].z), t);
                glm::quat rotation = glm::slerp(glm::quat(outputs[k].w, outputs[k].x, outputs[k].y, outputs[k].z),
                                                 glm::quat(outputs[k + 1].w, outputs[k + 1].x, outputs[k + 1].y, outputs[k + 1].z), t);
                glm::vec3 scale = glm::mix(glm::vec3(1.0f), glm::vec3(outputs[k + 1].w, outputs[k + 1].x, outputs[k + 1].y), t);

                // Construct the transformation matrix: Scale -> Rotate -> Translate
                glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
                glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
                glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);

                // Ensure the joint matrix is in the correct order for GLSL (column-major)
                jointMatrix = translationMatrix * rotationMatrix * scaleMatrix;

                break; // Exit the loop after processing the current joint
            }
        }

        // Store the joint transform, converting to column-major order
        // Note: glm::value_ptr is used to get the pointer for row-major ordering
        glm::mat4 colMajorJointMatrix = glm::transpose(jointMatrix);
        newBoneTransforms[jointNodeIndex] = colMajorJointMatrix; // Store the transpose for column-major layout
    }

    // Store all bone transforms in the buffer for use in the shader
    storeBoneTransforms(SSBO, newBoneTransforms.data());
}


//Globals for now, testing
GLuint SSBO = 0;
size_t JOINT_COUNT = 0;

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

    MODEL = model;

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
                //Vertex positions buffer
                const tinygltf::Accessor& positionAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
                const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];
                glBindBuffer(positionBufferView.target, vbo);
                glBufferData(positionBufferView.target, positionBufferView.byteLength,
                         &positionBuffer.data.at(0) + positionBufferView.byteOffset,
                         GL_STATIC_DRAW);
                glVertexAttribPointer(0, getNumberOfComponents(positionAccessor.type), positionAccessor.componentType, positionAccessor.normalized ? GL_TRUE : GL_FALSE, positionAccessor.ByteStride(positionBufferView), nullptr);
                glEnableVertexAttribArray(0);


                //Texture UVs buffer
                const tinygltf::Accessor& texCoordAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
                const tinygltf::Buffer& texCoordBuffer = model.buffers[texCoordBufferView.buffer];
                glBindBuffer(texCoordBufferView.target, texvbo);
                glBufferData(texCoordBufferView.target, texCoordBufferView.byteLength,
                         &texCoordBuffer.data.at(0) + texCoordBufferView.byteOffset,
                         GL_STATIC_DRAW);
                glVertexAttribPointer(1, getNumberOfComponents(texCoordAccessor.type), texCoordAccessor.componentType, texCoordAccessor.normalized ? GL_TRUE : GL_FALSE, texCoordAccessor.ByteStride(texCoordBufferView), nullptr);
                glEnableVertexAttribArray(1);

                //Boneids aka joints buffer
                const tinygltf::Accessor& boneIDAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
                const tinygltf::BufferView& boneIDBufferView = model.bufferViews[boneIDAccessor.bufferView];
                const tinygltf::Buffer& boneIDBuffer = model.buffers[boneIDBufferView.buffer];
                glBindBuffer(boneIDBufferView.target, boneidvbo);
                glBufferData(boneIDBufferView.target, boneIDBufferView.byteLength,
                         &boneIDBuffer.data.at(0) + boneIDBufferView.byteOffset,
                         GL_STATIC_DRAW);
                glVertexAttribPointer(2, getNumberOfComponents(boneIDAccessor.type), boneIDAccessor.componentType, boneIDAccessor.normalized ? GL_TRUE : GL_FALSE, boneIDAccessor.ByteStride(boneIDBufferView), nullptr);
                glEnableVertexAttribArray(2);

                //Weights buffer
                const tinygltf::Accessor& weightsAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
                const tinygltf::BufferView& weightsBufferView = model.bufferViews[weightsAccessor.bufferView];
                const tinygltf::Buffer& weightsBuffer = model.buffers[weightsBufferView.buffer];
                glBindBuffer(weightsBufferView.target, weightvbo);
                glBufferData(weightsBufferView.target, weightsBufferView.byteLength,
                         &weightsBuffer.data.at(0) + weightsBufferView.byteOffset,
                         GL_STATIC_DRAW);
                glVertexAttribPointer(3, getNumberOfComponents(positionAccessor.type), weightsAccessor.componentType, weightsAccessor.normalized ? GL_TRUE : GL_FALSE, weightsAccessor.ByteStride(weightsBufferView), nullptr);
                glEnableVertexAttribArray(3);


                //Indices buffer
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

    assert(model.skins.size() == 1); //just joking
    SSBO = createSSBOForBoneTransformsAndInvBindMats();
    tinygltf::Skin& skin = model.skins.at(0);
    const std::vector<int>& joints = skin.joints;
    const size_t numJoints = joints.size();
    JOINT_COUNT = numJoints;

    const tinygltf::Accessor& invBindMatricesAccessor = model.accessors[skin.inverseBindMatrices];
    const tinygltf::BufferView& invBindMatricesBufferView = model.bufferViews[invBindMatricesAccessor.bufferView];
    const tinygltf::Buffer& invBindMatricesBuffer = model.buffers[invBindMatricesBufferView.buffer];

    // Inverse bind matrices are stored as mat4 (16 floats)
    const float* invBindMatricesData = reinterpret_cast<const float*>(
        &invBindMatricesBuffer.data[invBindMatricesBufferView.byteOffset + invBindMatricesAccessor.byteOffset]);

    // Store the inverse bind matrices in a std::vector of glm::mat4
    std::vector<glm::mat4> inverseBindMatrices(numJoints);
    for (size_t i = 0; i < numJoints; ++i) {
        inverseBindMatrices[i] = glm::make_mat4(&invBindMatricesData[i * 16]);  // 16 floats per mat4
    }

    // Step 3: Get the current joint (bone) transforms
    std::vector<glm::mat4> boneTransforms(numJoints);
    for (size_t i = 0; i < numJoints; ++i) {
        int jointNodeIndex = joints[i];  // Index of the node in the model's node list
        const tinygltf::Node& jointNode = model.nodes[jointNodeIndex];

        glm::mat4 jointMatrix;

        // If the node has a matrix, use it
        if (!jointNode.matrix.empty()) {
            // Assuming jointNode.matrix is a std::vector<double> or similar type with 16 elements
            jointMatrix = glm::make_mat4(jointNode.matrix.data());
        } else {
            // Otherwise, compose the matrix from translation, rotation, and scale

            // Check if translation, rotation, and scale vectors are correctly populated
            glm::vec3 translation(0.0f);
            if (jointNode.translation.size() >= 3) {
                translation = glm::vec3(jointNode.translation[0],
                                         jointNode.translation[1],
                                         jointNode.translation[2]);
            }

            glm::vec3 scale(1.0f);
            if (jointNode.scale.size() >= 3) {
                scale = glm::vec3(jointNode.scale[0],
                                  jointNode.scale[1],
                                  jointNode.scale[2]);
            }

            glm::quat rotation = glm::quat(1, 0, 0, 0); // Identity rotation
            if (jointNode.rotation.size() >= 4) {
                rotation = glm::quat(jointNode.rotation[3], // w
                                     jointNode.rotation[0], // x
                                     jointNode.rotation[1], // y
                                     jointNode.rotation[2]); // z
            }

            // Construct the transformation matrix: Scale -> Rotate -> Translate
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0), scale);
            glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0), translation);

            // Note the order of multiplication
            jointMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        }

        // Store the joint transform (which may be animated later)
        boneTransforms[i] = jointMatrix;
    }

    storeBoneTransformsAndInvBindMats(SSBO, boneTransforms.data(), inverseBindMatrices.data());

    return ModelAndTextures{modelGLObjects, texids};
}

} // jl