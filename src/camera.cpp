#include "camera.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"

Camera::Camera(int width, int height)
    : m_width(width),
      m_height(height),
      m_global_up(0.0, 1.0, 0.0),
      m_forward(0.0, 0.0, 1.0),
      m_position(0.0, 0.0, 0.0),
      m_fov(glm::radians(45.0))
{
  compute();
}

Ray Camera::get_ray(int x, int y) const
{
  glm::dvec2 image_size(m_width, m_height);

  glm::dvec2 uv = (glm::dvec2(x, y) / image_size) * 2.0 - 1.0;

  Ray ray;
  ray.origin = m_position;

  double aspect_ratio = image_size.x / image_size.y;
  double half_height = std::tan(m_fov / 2.0);
  double half_width = half_height * aspect_ratio;

  double width = 2 * half_width;
  double height = 2 * half_height;

  glm::dvec3 target = m_position + m_forward;
  glm::dvec3 view_point = target + (width * m_right * uv.x) - (height * m_up * uv.y);

  ray.direction = glm::normalize(view_point - m_position);
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

void Camera::compute()
{
  m_right = glm::cross(m_forward, m_global_up);
  m_up = glm::cross(m_right, m_forward);
}

void Camera::look_at(const glm::dvec3& position, const glm::dvec3& target)
{
  m_position = position;
  m_forward = glm::normalize(target - position);
  compute();
}