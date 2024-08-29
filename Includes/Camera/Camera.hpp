//
// Created by wpsimon09 on 18/08/24.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GLFW/glfw3.h>


class Camera {
public:
    explicit Camera(GLFWwindow* window,glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                    float radius = 40.0f, float minRadius = 1.0f, float azimuthAngle = -10.0f,
                    float polarAngle = 10.0f);

    void rotateAzimutn(float radians);

    void rotatePolar(float radains);

    void zoom(float by);

    void moveHorizontal(float distance);

    void moveVertical(float distance);

    void processResize(int newWidht, int newHeight);

    glm::mat4 getPojectionMatix() const {return this->projection;}

    glm::mat4 getViewMatrix() const {return glm::lookAt(this->position,center, this->worldUp);};

    glm::vec3 getPosition() const {return this->position;};

    float getFarPlane() {return this->farPlane;}

    float getNearPlane() {return this->nearPlane;}

    bool getIsFirstMouse() const {return m_isFirstMouse;}

    void setIsFirstMouse(bool val = false ) {this->m_isFirstMouse = val;}

    void update();

    ~Camera() = default;

private:
    glm::vec3 getEye();

    float radius;
    float minRadius;
    float azimuthAngle;
    float polarAngle;

    glm::vec3 position;
    glm::vec3 center;
    glm::vec3 worldUp;


    glm::mat4 projection = glm::mat4(1.0f);

    float farPlane;
    float nearPlane;
    bool m_isFirstMouse = true;
};



#endif //CAMERA_HPP
