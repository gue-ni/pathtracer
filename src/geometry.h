#pragma once

#include "glm/glm.hpp"
#include "ray.h"

struct Intersection {
  bool hit;
  double t;
  glm::dvec3 point;
  glm::dvec3 normal;
};

struct Sphere {
  glm::dvec3 center;
  double radius;
};

struct Triangle {
  glm::dvec3 v[3];
};

struct Primitive {
  enum Type : uint8_t { SPHERE, TRIANGLE };
  union {
    Sphere sphere;
    Triangle triangle;
  };
};