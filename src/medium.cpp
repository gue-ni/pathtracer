
#include "util.h"
#include "medium.h"

glm::dvec3 Medium::transmittance(double d) const
{
  const glm::dvec3 coeff = m_absorbance_coeff + m_scatter_coeff;
  return glm::exp(-coeff * d);
}

Medium::Medium()
    : m_g(0), m_absorbance_coeff(0.001764, 0.0032095, 0.0019617), m_scatter_coeff(0.31845, 0.31324, 0.30147)

{
}

// Henyey and Greenstein
double Medium::phase_function(const glm::dvec3& wo, const glm::dvec3& wi) const
{
  double g2 = sq(m_g);
  double cos_theta = glm::dot(wo, wi);
  double inv_4pi = 1.0 / (4.0 * pi);
  return inv_4pi * ((1.0 - g2) / std::pow(1.0 + g2 + 2 * m_g * cos_theta, 3.0 / 2.0));
}

std::optional<ScatterEvent> Medium::interaction(double t) const
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(0, 1);
  int c = distr(gen);

  double e = random_double();

  const glm::dvec3 coeff = m_absorbance_coeff + m_scatter_coeff;
  double d = -std::log(1.0 - e) / coeff[c];

  return std::nullopt;  // TODO: fix
}
