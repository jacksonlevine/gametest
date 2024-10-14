//
// Created by jack on 10/13/2024.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace jl {

class Transform {
public:
    float yaw;
    float pitch;
    glm::vec3 direction;
    glm::vec3 position;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 velocity;
    Transform() :
        yaw(0.0),
        pitch(0.0),
        direction(glm::vec3(0.0, 0.0, 1.0)),
        position(glm::vec3(0.0, 0.0, 0.0)),
        right(glm::vec3(1.0, 0.0, 0.0)),
        up(glm::vec3(0.0, 1.0, 0.0)),
        velocity(glm::vec3(0.0, 0.0, 0.0))
    {}
    void updateWithYawPitch(float yaw, float pitch);
};

} // jl

#endif //TRANSFORM_H
