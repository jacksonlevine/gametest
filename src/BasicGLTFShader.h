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
            #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;

            out vec2 TexCoord;

            uniform mat4 mvp;
            uniform vec3 pos;

            void main()
            {
                TexCoord = inTexCoord;
                gl_Position = mvp * vec4(inPosition + pos, 1.0);
            }
        )glsl",
        R"glsl(
            #version 330 core
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
