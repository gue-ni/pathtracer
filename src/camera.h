
#pragma once

#include "ray.h"

class Camera
{
 public:
  Camera(int width, int height);
  Ray get_ray(int x, int y) const;
  int width() const;
  int height() const;
  void set_position(const glm::dvec3& position);
  void set_forward(const glm::dvec3& forward);

 private:
  const int m_width, m_height;
  const glm::dvec3 m_up;
  glm::dvec3 m_position, m_forward, m_right;

  void compute();
};