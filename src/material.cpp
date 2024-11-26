#include "material.h"
#include <cassert>
#include <cmath>
#include "geometry.h"
#include "util.h"

#define PT_IMPORTANCE_SAMPLE 0

/*
Links:
https://agraphicsguynotes.com/posts/sample_microfacet_brdf/
https://computergraphics.stackexchange.com/questions/7656/importance-sampling-microfacet-ggx
*/

static double AbsDot(const glm::dvec3& a, const glm::dvec3& b) { return glm::abs(glm::dot(a, b)); }

static double SameHemisphere(const glm::dvec3& a, const glm::dvec3& b) { return glm::dot(a, b) > 0.0; }

static double CosTheta(const glm::dvec3& a)
{
#if 0
  return glm::abs(a.y);
#else
  return a.y;
#endif
}

static glm::dvec3 SchlickFresnel(const glm::dvec3& f0, double radians)
{
  return f0 + (glm::dvec3(1.0) - f0) * std::pow(1.0 - radians, 5.0);
}

static double D_GGX(double NoH, double roughness)
{
  double alpha = sq(roughness);
  double alpha2 = sq(alpha);
  double NoH2 = sq(NoH);
  double b = (NoH2 * (alpha2 - 1.0) + 1.0);
  return alpha2 * (1 / pi) / (b * b);
}

static double D_Beckmann(double NoH, double roughness)
{
  double alpha = sq(roughness);
  double alpha2 = sq(alpha);
  double NoH2 = sq(NoH);
  double NoH4 = sq(NoH2);
  double sinTheta = std::sqrt(1.0 - NoH2);
  double tanTheta = sinTheta / NoH;
  return std::exp((NoH2 - 1.0) / (alpha2 * NoH2)) / (alpha2 * pi * NoH4);
}

static glm::dvec3 Sample_Beckmann(const glm::dvec3& V, double roughness)
{
  double alpha = sq(roughness);
  double alpha2 = sq(alpha);
  double e0 = random_double(), e1 = random_double();
  double phi = 2.0 * pi * e0;
  double theta = std::atan(std::sqrt(-alpha2 * std::log(e1)));
  glm::dvec3 H = spherical_to_cartesian(theta, phi);
  return glm::reflect(-V, H);
}

static double PDF_Beckmann(double NoH, double roughness)
{
  // double alpha = sq(roughness);
  double alpha2 = sq(roughness);
  double NoH2 = sq(NoH);
  double sinTheta = std::sqrt(1.0 - NoH2);
  double tanTheta = sinTheta / NoH;
  double NoH3 = NoH * NoH * NoH;
  double a = 1.0 / (pi * alpha2 * NoH3);
  double b = std::exp(-sq(sinTheta / NoH) / alpha2);
  return a * b;
}

static double G1_GGX_Schlick(double NoV, double roughness)
{
  double alpha = roughness * roughness;
  double k = alpha / 2.0;
  return glm::max(NoV, 0.001) / (NoV * (1.0 - k) + k);
}

static double G_Smith(double NoV, double NoL, double roughness)
{
  return G1_GGX_Schlick(NoV, roughness) * G1_GGX_Schlick(NoL, roughness);
}

static double SmithGGXMaskingShadowing(double NoL, double NoV, double a2)
{
  double denomA = NoV * glm::sqrt(a2 + (1.0 - a2) * sq(NoL));
  double denomB = NoL * glm::sqrt(a2 + (1.0 - a2) * sq(NoV));
  return (2.0 * NoL * NoV) / (denomA + denomB);
}

static double reflectance(double cosine, double refraction_index)
{
  // Use Schlick's approximation for reflectance.
  auto r0 = (1 - refraction_index) / (1 + refraction_index);
  r0 = r0 * r0;
  return r0 + (1 - r0) * std::pow((1 - cosine), 5);
}

BxDF::BxDF(Intersection const* const s) : surface(s) {}

glm::dvec3 BxDF::sample(const glm::dvec3& wo) const
{
  switch (surface->material->type) {
    case Material::SPECULAR:
      return sample_specular(wo);
    case Material::MICROFACET:
      return sample_microfacet(wo);
    case Material::DIELECTRIC:
      return sample_dielectric(wo);
    default:
      return sample_diffuse(wo);
  }
}

glm::dvec3 BxDF::eval(const glm::dvec3& wo, const glm::dvec3& wi) const
{
  switch (surface->material->type) {
    case Material::SPECULAR:
      return eval_specular(wo, wi);
    case Material::MICROFACET:
      return eval_microfacet(wo, wi);
    case Material::DIELECTRIC:
      return eval_dielectric(wo, wi);
    default:
      return eval_diffuse(wo, wi);
  }
}

glm::dvec3 BxDF::sample_diffuse(const glm::dvec3& wo) const
{
  double e0 = random_double(), e1 = random_double();
  double phi = 2.0 * pi * e0;
  double theta = std::acos(std::sqrt(e1));
  return spherical_to_cartesian(theta, phi);
}

glm::dvec3 BxDF::eval_diffuse(const glm::dvec3& wo, const glm::dvec3& wi) const
{
#if 0
  double cos_theta = wi.y;
  double pdf = cos_theta / pi;
  return ((surface->albedo() / pi)) * cos_theta / pdf;
#else
  return surface->albedo();
#endif
}

glm::dvec3 BxDF::sample_specular(const glm::dvec3& wo) const
{
  double fuzz = surface->material->roughness;
  return glm::reflect(-wo, glm::dvec3(0, 1, 0)) + (fuzz * random_unit_vector());
}

glm::dvec3 BxDF::eval_specular(const glm::dvec3& wo, const glm::dvec3& wi) const { return surface->albedo(); }

glm::dvec3 BxDF::sample_microfacet(const glm::dvec3& wo) const
{
#if PT_IMPORTANCE_SAMPLE
  return Sample_Beckmann(wo, surface->material->roughness);
#else
  return sample_diffuse(wo);
#endif
}

glm::dvec3 BxDF::eval_microfacet(const glm::dvec3& V, const glm::dvec3& L) const
{
  if (!SameHemisphere(V, L)) {
    return glm::dvec3(0);
  }

  glm::dvec3 base_color = surface->albedo();
  double metallic = surface->material->metallic;
  double roughness = surface->material->roughness;

  double alpha = sq(roughness);
  double alpha2 = sq(alpha);

  glm::dvec3 N = glm::dvec3(0, 1, 0);
  glm::dvec3 H = glm::normalize(V + L);

  double NoV = CosTheta(V);
  double NoL = CosTheta(L);
  double NoH = CosTheta(V);
  double VoH = AbsDot(V, H);

  glm::dvec3 f0 = glm::mix(glm::dvec3(0.04), base_color, metallic);

  glm::dvec3 F = SchlickFresnel(f0, VoH);
  double G = G_Smith(NoV, NoL, roughness);
  double D = D_Beckmann(NoH, roughness);

  glm::dvec3 specular = (F * G * D) / (4.0 * NoV * NoL);

  glm::dvec3 diffuse = (glm::dvec3(1.0) - F) * base_color / pi;

#if PT_IMPORTANCE_SAMPLE
  double pdf = PDF_Beckmann(NoH, roughness);
#else
  double pdf = NoL / pi;
#endif

  glm::dvec3 brdf_value = specular;

  return (brdf_value * NoL) / pdf;
}

glm::dvec3 BxDF::sample_mirror(const glm::dvec3& wo) const { return glm::reflect(-wo, glm::dvec3(0, 1, 0)); }

glm::dvec3 BxDF::eval_mirror(const glm::dvec3& wo, const glm::dvec3& wi) const { return surface->albedo(); }

glm::dvec3 BxDF::sample_dielectric(const glm::dvec3& wo) const
{
  double refraction_index = surface->material->refraction_index;
  double ri = surface->inside ? (1.0 / refraction_index) : refraction_index;

  glm::dvec3 normal(0, 1, 0);

  double cos_theta = glm::min(wo.y, 1.0);
  double sin_theta = std::sqrt(1.0 - sq(cos_theta));

  if ((ri * sin_theta > 1.0) || (reflectance(cos_theta, ri) > random_double())) {
    return glm::reflect(-wo, normal);
  } else {
    return glm::refract(-wo, normal, ri);
  }
}

glm::dvec3 BxDF::eval_dielectric(const glm::dvec3& wo, const glm::dvec3& wi) const { return glm::dvec3(1); }
