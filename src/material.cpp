#include "material.h"
#include <cassert>
#include <cmath>
#include "geometry.h"
#include "util.h"

/*
Links:
https://agraphicsguynotes.com/posts/sample_microfacet_brdf/
*/

static double abs_dot(const glm::dvec3& a, const glm::dvec3& b) { return glm::clamp(glm::dot(a, b), 0.0, 1.0); }

static double SameHemisphere(const glm::dvec3& a, const glm::dvec3& b) { return glm::dot(a, b) > 0.0; }

static double CosTheta(const glm::dvec3& a) { return a.y; }

// https://computergraphics.stackexchange.com/questions/7656/importance-sampling-microfacet-ggx
static glm::dvec3 SchlickFresnel(const glm::dvec3& f0, double radians)
{
  return f0 + (glm::dvec3(1.0) - f0) * std::pow(1.0 - radians, 5.0);
}

static double D_GGX(double NoH, double roughness)
{
  double alpha = roughness * roughness;
  double alpha2 = alpha * alpha;
  double NoH2 = NoH * NoH;
  double b = (NoH2 * (alpha2 - 1.0) + 1.0);
  return alpha2 * (1 / pi) / (b * b);
}

static double D_Beckmann(double NoH, double roughness)
{
  double alpha = sq(roughness);
  double alpha2 = sq(alpha);
  double NoH4 = (NoH * NoH * NoH * NoH);

  return (1.0 / (pi * alpha2 * NoH4)) * std::exp(-());
}

static glm::dvec3 Sample_Beckmann()
{
  return glm::dvec3();
}

static double PDF_Beckmann() { return 0; }

static double D_Blinn_Phong(double NoH, double roughness)
{
  double alpha = sq(roughness);
  return ((alpha + 2) * std::pow(NoH, alpha)) / (2.0 * pi);
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

// https://www.youtube.com/watch?v=gya7x9H3mV0
static glm::dvec3 microfacet_brdf(const glm::dvec3& L, const glm::dvec3& V, const glm::dvec3& N,
                                  const glm::dvec3& base_color, double reflectance, double metallic, double roughness

)
{
  glm::dvec3 H = glm::normalize(V + L);

  double NoV = glm::clamp(glm::dot(N, V), 0.0, 1.0);
  double NoL = glm::clamp(glm::dot(N, L), 0.0, 1.0);
  double NoH = glm::clamp(glm::dot(N, H), 0.0, 1.0);
  double VoH = glm::clamp(glm::dot(V, H), 0.0, 1.0);

#if 0
  glm::dvec3 f0 = glm::vec3(0.16 * (reflectance * reflectance));
  f0 = glm::mix(f0, base_color, metallic);
#else
  glm::dvec3 f0(0.04);
  f0 = glm::mix(f0, base_color, metallic);
#endif

  glm::dvec3 F = SchlickFresnel(f0, VoH);
  double D = D_GGX(NoH, roughness);
  double G = G_Smith(NoV, NoL, roughness);

  glm::dvec3 specular = (F * D * G) / (4.0 * glm::max(NoV, 0.001) * glm::max(NoL, 0.001));

  glm::dvec3 rhoD = base_color;
  rhoD *= glm::dvec3(1.0) - F;
  rhoD *= (1.0 - metallic);
  glm::dvec3 diffuse = rhoD * (1.0 / pi);

  return diffuse + specular;
}

static double SmithGGXMaskingShadowing(double NoL, double NoV, double a2)
{
  return 1;
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

#define IMPORTANCE_SAMPLE 0

glm::dvec3 BxDF::sample_microfacet(const glm::dvec3& wo) const
{
#if IMPORTANCE_SAMPLE
  // Beckmann
  double alpha = sq(surface->material->roughness);
  double e0 = random_double(), e1 = random_double();
  double phi = 2 * pi * e0;
  double theta = std::atan(-alpha * std::log(e1));
  glm::dvec3 wm = spherical_to_cartesian(theta, phi);
  return glm::reflect(-wo, wm);
#else
  return sample_diffuse(wo);
#endif
}

glm::dvec3 BxDF::eval_microfacet(const glm::dvec3& wo, const glm::dvec3& wi) const
{
  glm::dvec3 base_color = surface->albedo();
  double metallic = surface->material->metallic;
  double roughness = surface->material->roughness;

  double alpha = sq(roughness);
  double alpha2 = sq(alpha);

  glm::dvec3 n = glm::dvec3(0, 1, 0);
  glm::dvec3 wm = glm::normalize(wo + wi);

  double n_dot_wo = wo.y;
  double n_dot_wi = wi.y;
  double n_dot_wm = wm.y;
  double n_dot_wm2 = sq(n_dot_wm);
  double wo_dot_wm = glm::dot(wo, wm);
  double cos_theta = n_dot_wi;

  glm::dvec3 f0 = glm::mix(glm::dvec3(0.04), base_color, metallic);

  glm::dvec3 F = SchlickFresnel(f0, wo_dot_wm);

  double G = G_Smith(n_dot_wo, n_dot_wi, roughness);

  // Beckmann
  // let d = ((nh2 - 1.0) / (m2 * nh2)).exp() / (m2 * glm::pi::<f64>() * nh2 * nh2);

  double D = std::exp((n_dot_wm2 - 1.0) / (alpha2 * n_dot_wm2)) / (alpha2 * sq(n_dot_wm2));

  glm::dvec3 specular = (F * G * D) / (4.0 * n_dot_wo * n_dot_wi);

  glm::dvec3 diffuse = (glm::dvec3(1.0) - F) * base_color / pi;

#if IMPORTANCE_SAMPLE
  double cos_t = n_dot_wm;
  double sin_t = std::sqrt(1.0 - sq(cos_t));
  double pdf = (1.0 / (pi * alpha * cb(cos_t))) * std::exp(-sq(sin_t / cos_t) / alpha);
#else
  double pdf = cos_theta / pi;
#endif

  return ((specular + diffuse) * cos_theta) / pdf;
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
