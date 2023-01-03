#ifndef __CAMERA_H__
#define __CAMERA_H__

namespace Ths::engine
{
  struct Camera
  {
    glm::vec3 translation = {-2.8f, 0, 2.0f};
    glm::vec3 focalPoint = {0.0f, 0.0f, 0.5f};
    glm::vec3 upDirection = {0.0f, 0.0f, 1.0f};
    float fov = 60.0f;
    float nearClippingPlane = 0.1f;
    float farClippingPlane = 100.0f;
  };
}

#endif // __CAMERA_H__