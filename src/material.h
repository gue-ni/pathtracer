#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "glm/fwd.hpp"

#include "ray.h"

struct Intersection;

struct Material {
  enum Type : uint8_t {
    DIFFUSE,
    SPECULAR,
  };
  Type type = DIFFUSE;
  glm::dvec3 albedo;
  glm::dvec3 emittance;
  Material() : Material(glm::dvec3(0.0), glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a) : Material(a, glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a, const glm::dvec3& r) : albedo(a), emittance(r) {}
};

class BRDF
{
 public:
  BRDF(Intersection*);
  std::tuple<Ray, glm::dvec3> sample(const Ray& incoming);

 private:
  Intersection* surface;
  std::tuple<Ray, glm::dvec3> sample_diffuse(const Ray& incoming);
  std::tuple<Ray, glm::dvec3> sample_specular(const Ray& incoming);
};