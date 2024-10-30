#include "camera.h"

Camera::Camera(int width, int height)
    : m_width(width), m_height(height), m_up(0.0, 1.0, 0.0), m_forward(0.0, 0.0, 1.0), m_position(0.0, 0.0, 0.0)
{
  compute();
}

Ray Camera::get_ray(int x, int y) const
{
  double width = double(m_width);
  double height = double(m_height);

  double u = double(x) / double(m_width);
  double v = double(y) / double(m_height);

  Ray ray;
  ray.origin = m_position;

  double aspect_ratio = width / height;
  // double half_width = std::tan()

  // glm::dvec3 target = m_position + ;

  // ray.direction = glm::normalize(target - m_position);
  return ray;
}

int Camera::width() const { return m_width; }

int Camera::height() const { return m_height; }

void Camera::set_position(const glm::dvec3& position)
{
  m_position = position;
  compute();
}

void Camera::set_forward(const glm::dvec3& forward)
{
  m_forward = forward;
  compute();
}

void Camera::compute() { m_right = glm::cross(m_forward, m_up); }
