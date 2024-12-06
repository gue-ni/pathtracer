#pragma once

#include "ray.h"
#include <optional>
#include <glm/glm.hpp>

struct ScatterEvent {
  double t;
  Ray ray;
};

class Medium
{
 public:
  Medium();
  double phase_function(const glm::dvec3& wo, const glm::dvec3& wi) const;
  glm::dvec3 transmittance(double d) const;
  std::optional<ScatterEvent> interaction(double t) const;

 private:
  const double m_g;
  const glm::dvec3 m_scatter_coeff;
  const glm::dvec3 m_absorbance_coeff;
};