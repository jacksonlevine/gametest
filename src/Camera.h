//
// Created by jack on 10/13/2024.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "Transform.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace jl {

class Camera {
public:
    jl::Transform transform;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 mvp;
    Camera()
        : model(glm::mat4(1.0f)),
          view(glm::mat4(1.0f)),
          projection(glm::mat4(1.0f)),
          mvp(glm::mat4(1.0f))
        {}
    void updateProjection(int screenwidth, int screenheight, float fov);
    void updateWithYawPitch(float nyaw, float npitch);
};

} // jl

#endif //CAMERA_H
