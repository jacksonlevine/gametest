//
// Created by jack on 10/13/2024.
//

#ifndef BASICGLTFSHADER_H
#define BASICGLTFSHADER_H
#include "Shader.h"


jl::Shader getBasicGLTFShader();

inline jl::Shader getBasicGLTFShader()
{
    jl::Shader shader(
        R"glsl(
            #version 430 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;
            layout(location = 2) in ivec4 boneIDs;
            layout(location = 3) in vec4 weights;

            layout(std430, binding = 0) buffer BoneDataBuffer {
                mat4 boneTransforms[500];
                mat4 inverseBindMatrices[500];
            };

            out vec2 TexCoord;

            uniform mat4 mvp;
            uniform vec3 pos;
            uniform int jointCount;

            void main() {
                vec4 finalPosition = vec4(0.0);


                //loop over the bone IDs and accumulate weighted bone transforms
                for (int i = 0; i < 4; ++i) {
                    int boneId = boneIDs[i];
                    float weight = weights[i];
                    mat4 boneTransform = boneTransforms[boneId] * inverseBindMatrices[boneId];
                    if (boneId >= jointCount) {
                        boneTransform = mat4(1.0);
                    }

                    finalPosition += weight * (boneTransform * vec4(inPosition, 1.0));
                }

                gl_Position = mvp * (finalPosition + vec4(pos, 0.0));
                TexCoord = inTexCoord;
            }
        )glsl",
        R"glsl(
            #version 430 core
            out vec4 FragColor;

            in vec2 TexCoord;

            uniform sampler2D texture1;

            void main()
            {
                FragColor = texture(texture1, TexCoord);
            }
        )glsl",
        "basicGLTFShader"

        );
    return shader;
}


#endif //BASICGLTFSHADER_H
