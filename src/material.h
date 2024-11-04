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
  Material() : Material(DIFFUSE, glm::dvec3(0.0), glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a) : Material(DIFFUSE, a, glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a, const glm::dvec3& r) : type(DIFFUSE), albedo(a), emittance(r) {}
  Material(Type t, const glm::dvec3& a, const glm::dvec3& r) : type(t), albedo(a), emittance(r) {}
};

class BRDF
{
 public:
  struct Sample {
    Ray ray;
    glm::dvec3 value;
  };
  BRDF(Intersection*);
  Sample sample(const Ray& incoming);

 private:
  Intersection* surface;
  Sample sample_diffuse(const Ray& incoming);
  Sample sample_specular(const Ray& incoming);
};