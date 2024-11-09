
#pragma once

#include "ray.h"

class Camera
{
 public:
  Camera(int width, int height, double fov, double aperture, double focus_distance);
  Ray get_ray(int x, int y) const;
  int width() const;
  int height() const;
  void set_position(const glm::dvec3& position);
  void set_forward(const glm::dvec3& forward);
  void look_at(const glm::dvec3& position, const glm::dvec3& target);
  glm::dvec3 position() const { return m_position; }
  glm::dvec3 direction() const { return m_forward; }

 private:
  const int m_width, m_height;
  const glm::dvec3 m_world_up;
  glm::dvec3 m_position, m_forward, m_right, m_up;
  double m_fov;
  double m_focus_distance;
  double m_aperture ;


  void compute();
};