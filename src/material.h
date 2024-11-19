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
  glm::dvec3 emission;
  double refraction_index = 1.52;
  double shininess = 1000;  // range [0, 1000]
  double roughness = 0.5;   //
  double metallic = 0.5;    // 0.0 for dielectrics, 1.0 for metals

  Material() : Material(DIFFUSE, glm::dvec3(0.0), glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a) : Material(DIFFUSE, a, glm::dvec3(0.0)) {}
  Material(const glm::dvec3& a, const glm::dvec3& r) : type(DIFFUSE), albedo(a), emission(r) {}
  Material(Type t, const glm::dvec3& a, const glm::dvec3& r) : type(t), albedo(a), emission(r) {}
};

class BxDF
{
 public:
  BxDF(Intersection*);
  glm::dvec3 sample(const glm::dvec3& wo) const;
  glm::dvec3 eval(const glm::dvec3& wo, const glm::dvec3& wi) const;

 private:
  Intersection* surface;

  glm::dvec3 sample_diffuse(const glm::dvec3& wo) const;
  glm::dvec3 eval_diffuse(const glm::dvec3& wo, const glm::dvec3& wi) const;

  glm::dvec3 sample_specular(const glm::dvec3& wo) const;
  glm::dvec3 eval_specular(const glm::dvec3& wo, const glm::dvec3& wi) const;

  glm::dvec3 sample_dielectric(const glm::dvec3& wo) const;
  glm::dvec3 eval_dielectric(const glm::dvec3& wo, const glm::dvec3& wi) const;
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
  Sample sample_mirror(const Ray& incoming);
  Sample sample_specular(const Ray& incoming);
  Sample sample_microfacet(const Ray& incoming);
  Sample sample_microfacet_2(const Ray& incoming);
  Sample sample_transmissive(const Ray& incoming);
};
