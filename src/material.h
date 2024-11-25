#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include "ray.h"
#include "image.h"

struct Intersection;

struct Material {
  enum Type : uint8_t { DIFFUSE = 1 << 0, SPECULAR = 1 << 1, DIELECTRIC = 1 << 2, MIRROR = 1 << 3 };

  Type type = DIFFUSE;
  glm::dvec3 albedo = glm::dvec3(0.8);
  glm::dvec3 emission = glm::dvec3(0.0);
  double refraction_index = 1.52;
  double shininess = 1000;  // range [0, 1000]
  double roughness = 0.5;   //
  double metallic = 0.5;    // 0.0 for dielectrics, 1.0 for metals
  Image* texture = nullptr;

  inline bool is_perfectly_specular() const
  {
    return (type & Material::DIELECTRIC) || ((type & Material::SPECULAR) && (roughness < 0.01));
  }
};

// wo and wi are in local tangent space, so relative to the normal (0, 1, 0)
// wo is the direction towards the camera
// wi is the direction towards the light
class BxDF
{
 public:
  BxDF(Intersection const* const);
  glm::dvec3 sample(const glm::dvec3& wo) const;
  glm::dvec3 eval(const glm::dvec3& wo, const glm::dvec3& wi) const;

 private:
  Intersection const* const surface;

  glm::dvec3 sample_diffuse(const glm::dvec3& wo) const;
  glm::dvec3 eval_diffuse(const glm::dvec3& wo, const glm::dvec3& wi) const;

  glm::dvec3 sample_specular(const glm::dvec3& wo) const;
  glm::dvec3 eval_specular(const glm::dvec3& wo, const glm::dvec3& wi) const;

  glm::dvec3 sample_microfacet(const glm::dvec3& wo) const;
  glm::dvec3 eval_microfacet(const glm::dvec3& wo, const glm::dvec3& wi) const;

  glm::dvec3 sample_mirror(const glm::dvec3& wo) const;
  glm::dvec3 eval_mirror(const glm::dvec3& wo, const glm::dvec3& wi) const;

  glm::dvec3 sample_dielectric(const glm::dvec3& wo) const;
  glm::dvec3 eval_dielectric(const glm::dvec3& wo, const glm::dvec3& wi) const;
};
