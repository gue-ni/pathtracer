#pragma once

#include <glm/glm.hpp>

struct Ray {
  glm::dvec3 origin;
  glm::dvec3 direction;
  inline glm::dvec3 point_at(double t) const { return origin + direction * t; }
};