#pragma once

#include <glm/glm.hpp>
#include "glm/fwd.hpp"

#include "ray.h"

struct BRDF {
  Ray sample(Ray&);
  glm::dvec3 evaluate(Ray& wo);
  double pdf(Ray& wo);
};

struct Material {
  glm::dvec3 radiance;
  glm::dvec3 albedo;
  Material() : Material(glm::dvec3(0.0), glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a) : Material(a, glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a, const glm::dvec3& r) : albedo(a), radiance(r) {}
};