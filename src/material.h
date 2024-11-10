#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "ray.h"
#include "image.h"

struct Intersection;

struct Material {
  enum Type : uint8_t {
    DIFFUSE,
    SPECULAR,
    TRANSMISSIVE,
  };

  Type type = DIFFUSE;
  glm::dvec3 albedo;
  Image* texture = nullptr;
  glm::dvec3 emittance;
  double refraction_index = 1.52;
  double shininess = 1000;  // range [0, 1000]
  double roughness = 0.5;   //
  double metallic = 0.5;    // 0.0 for dielectrics, 1.0 for metals

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
  Sample sample_transmissive(const Ray& incoming);
};