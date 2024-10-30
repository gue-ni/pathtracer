#pragma once

#include "glm/glm.hpp"
#include "ray.h"
#include "material.h"
#include <optional>

struct Intersection {
  bool hit;
  double t;
  glm::dvec3 point;
  glm::dvec3 normal;
  Material* material;
};

struct Sphere {
  glm::dvec3 center;
  double radius;
};

bool ray_vs_sphere(const Ray&, const Sphere&, float& t);

struct Triangle {
  glm::dvec3 v[3];
};

bool ray_vs_triangle(const Ray&, const Triangle&, float& t);

struct Primitive {
  enum Type : uint8_t { SPHERE, TRIANGLE };
  Type type = SPHERE;
  union {
    Sphere sphere;
    Triangle triangle;
  };
  Material* material = nullptr;

  std::optional<Intersection> intersect(const Ray&) const;
};