#include "camera.h"

Camera::Camera(int width, int height)
    : m_width(width), m_height(height), m_up(0.0, 1.0, 0.0) {}

Ray Camera::get_ray(int x, int y) const { return Ray(); }

int Camera::width() const { return m_width; }

int Camera::height() const { return m_height; }
