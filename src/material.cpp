#include "material.h"
#include <cassert>
#include <cmath>
#include "geometry.h"
#include "util.h"

static double abs_dot(const glm::dvec3& a, const glm::dvec3& b) { return glm::clamp(glm::dot(a, b), 0.0, 1.0); }

static double SameHemisphere(const glm::dvec3& a, const glm::dvec3& b) { return glm::dot(a, b) > 0.0; }

static double CosTheta(const glm::dvec3& a) { return a.y; }

// Cook-Torrance Microface BRDF
// https://computergraphics.stackexchange.com/questions/7656/importance-sampling-microfacet-ggx
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static glm::dvec3 SchlickFresnel(const glm::dvec3& f0, double radians)
{
  return f0 + (glm::dvec3(1.0) - f0) * std::pow(1.0 - radians, 5.0);
}

static double D_GGX(double NoH, double roughness)
{
  return 1;
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

  return 0;
}

static double PDF_Beckmann() { return 0; }

static double D_Blinn_Phong(double NoH, double roughness)
{
  double alpha = sq(roughness);
  return ((alpha + 2) * std::pow(NoH, alpha)) / (2.0 * pi);
}

#if 0
static std::tuple<double, double, double> Sample_BlinnPhong(double NoH, double roughness)
{
  double alpha = sq(roughness);
  double e0 = random_double(), e1 = random_double();
  double phi = 2 * pi * e0;
  double theta = std::acos(std::pow(e1, (1 / (alpha + 2))));

  double pdf = ((alpha + 2) std::pow(NoH, alpha)) / (2 * pi);
}
#endif

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

//====================================================================
// non height-correlated masking-shadowing function is described here:
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BxDF::BxDF(Intersection* s) : surface(s) {}

glm::dvec3 BxDF::sample(const glm::dvec3& wo) const
{
  switch (surface->material->type) {
    case Material::SPECULAR:
      return sample_mirror(wo);
    case Material::TRANSMISSIVE:
      return sample_dielectric(wo);
    default:
      return sample_diffuse(wo);
  }
}

glm::dvec3 BxDF::eval(const glm::dvec3& wo, const glm::dvec3& wi) const
{
  switch (surface->material->type) {
    case Material::SPECULAR:
      return eval_mirror(wo, wi);
    case Material::TRANSMISSIVE:
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

glm::dvec3 BxDF::sample_mirror(const glm::dvec3& wo) const { 
  return glm::reflect(-wo, glm::dvec3(0, 1, 0));
}

glm::dvec3 BxDF::eval_mirror(const glm::dvec3& wo, const glm::dvec3& wi) const { 
  return surface->albedo();
}

glm::dvec3 BxDF::sample_dielectric(const glm::dvec3& wo) const
{
  double refraction_index = surface->material->refraction_index;
  double ri = surface->inside ? (1.0 / refraction_index) : refraction_index;

  double cos_theta = glm::min(wo.y, 1.0);
  double sin_theta = std::sqrt(1.0 - sq(cos_theta));

  glm::dvec3 normal(0, 1, 0);

  if ((ri * sin_theta > 1.0) || (reflectance(cos_theta, ri) > random_double())) {
    return glm::reflect(-wo, normal);
  } else {
    return glm::refract(-wo, normal, ri);
  }
}

glm::dvec3 BxDF::eval_dielectric(const glm::dvec3& wo, const glm::dvec3& wi) const { return glm::dvec3(1); }

BRDF::BRDF(Intersection* s) : surface(s) {}

// L = Le + (1/N) * ∑ (Li * brdf * cos(theta) * 1/pdf())
BRDF::Sample BRDF::sample(const Ray& incoming)
{
  switch (surface->material->type) {
    case Material::SPECULAR:
#if 0
      return (surface->material->roughness == 0) ? sample_mirror(incoming) : sample_specular(incoming);
#else
      return (surface->material->roughness == 0) ? sample_mirror(incoming) : sample_microfacet(incoming);
#endif
    case Material::TRANSMISSIVE:
      return sample_transmissive(incoming);
    default:
      return sample_diffuse(incoming);
  }
}

BRDF::Sample BRDF::sample_diffuse(const Ray& incoming)
{
  Ray outgoing = Ray(surface->point, cosine_weighted_sampling(surface->normal));
  double cos_theta = glm::max(glm::dot(surface->normal, outgoing.direction), 0.0);
  double pdf = cos_theta / pi;

  glm::dvec3 albedo = surface->albedo();
  return BRDF::Sample{outgoing, (albedo / pi) * cos_theta / pdf};
}

BRDF::Sample BRDF::sample_mirror(const Ray& incoming)
{
  Ray outgoing(surface->point, glm::reflect(incoming.direction, surface->normal));
  return BRDF::Sample{outgoing, surface->albedo()};
}

BRDF::Sample BRDF::sample_microfacet_2(const Ray& incoming)
{
  glm::mat3 transform = local_to_world(surface->normal);
#if 1
  double e0 = random_double(), e1 = random_double();
  double phi = 2.0 * pi * e0;
  double theta = std::acos(std::sqrt(e1));
  glm::dvec3 L = spherical_to_cartesian(theta, phi);
  double cos_theta = L.y;
  double pdf = cos_theta / pi;
#else
  double alpha = sq(surface->material->roughness);
  double e0 = random_double(), e1 = random_double();
  double phi = 2 * pi * e0;
  double theta = std::acos(std::pow(e1, (1.0 / (alpha + 2.0))));

  double cos_theta = std::cos(theta);

  double pdf = ((alpha + 2) * std::pow(cos_theta, alpha)) / (2 * pi);

  glm::dvec3 L = spherical_to_cartesian(theta, phi);

#endif

  Ray outgoing(surface->point, transform * L);

  glm::dvec3 value = microfacet_brdf(outgoing.direction, -incoming.direction, surface->normal, surface->albedo(), 1.0,
                                     surface->material->metallic, surface->material->roughness);

  return BRDF::Sample{outgoing, value};
}

// https://schuttejoe.github.io/post/ggximportancesamplingpart1/
BRDF::Sample BRDF::sample_microfacet(const Ray& incoming)
{
  double metallic = glm::clamp(surface->material->metallic, 0.001, 0.999);
  double roughness = glm::clamp(surface->material->roughness, 0.001, 0.999);

  // coordinate transformations
  glm::mat3 transform = local_to_world(surface->normal);
  glm::mat3 inverse_transform = glm::inverse(transform);

  glm::dvec3 V = inverse_transform * (-incoming.direction);
  glm::dvec3 N = inverse_transform * surface->normal;

  double e0 = random_double(), e1 = random_double();

  double a = sq(roughness);
  double a2 = sq(a);

  double phi = 2.0 * pi * e1;
  double theta = std::acos(std::sqrt((1.0 - e0) / ((a2 - 1.0) * e0 + 1.0)));

  // why is this so slow?
  // this causes much more bounces
  glm::dvec3 H = spherical_to_cartesian(theta, phi);
  glm::dvec3 L = 2.0 * glm::dot(V, H) * H - V;

  if (L.y > 0 && glm::dot(L, H) > 0) {
    double VoH = glm::dot(V, H);
    // since we are in tangent space, dot product with the normal
    // is just the y axis
    double NoL = L.y;
    double NoV = V.y;
    double NoH = H.y;

    glm::dvec3 f0 = glm::mix(glm::dvec3(0.04), surface->albedo(), metallic);
    glm::dvec3 F = SchlickFresnel(f0, VoH);
    double G = SmithGGXMaskingShadowing(NoL, NoV, a2);

    double weight = glm::abs(glm::dot(V, H)) / (NoV * NoH);

    glm::dvec3 rhoD = surface->albedo();
    rhoD *= glm::dvec3(1.0) - F;
    rhoD *= (1.0 - metallic);
    glm::dvec3 diffuse = rhoD * (1.0 / pi);

    glm::dvec3 value = (F * G * weight) + diffuse;

    Ray outgoing(surface->point, transform * L);
    return BRDF::Sample{outgoing, value};
  } else {
    Ray outgoing(surface->point, transform * L);
    return BRDF::Sample{outgoing, glm::dvec3(0)};
  }
}

BRDF::Sample BRDF::sample_specular(const Ray& incoming)
{
  glm::dvec3 reflected = glm::reflect(incoming.direction, surface->normal);
  Ray outgoing = Ray(surface->point, reflected);

  double fuzz = surface->material->roughness;
  outgoing.direction += (fuzz * random_unit_vector());

  glm::dvec3 albedo = surface->albedo();
  return BRDF::Sample{outgoing, albedo};
}

BRDF::Sample BRDF::sample_transmissive(const Ray& incoming)
{
  double refraction_index = surface->material->refraction_index;
  double ri = surface->inside ? (1.0 / refraction_index) : refraction_index;

  double cos_theta = glm::min(glm::dot(-incoming.direction, surface->normal), 1.0);
  double sin_theta = std::sqrt(1.0 - sq(cos_theta));

  Ray outgoing;
  outgoing.origin = surface->point;

  bool total_internal_refraction = ri * sin_theta > 1.0;

  if (total_internal_refraction || reflectance(cos_theta, ri) > random_double()) {
    outgoing.direction = glm::reflect(incoming.direction, surface->normal);
  } else {
    outgoing.direction = glm::refract(incoming.direction, surface->normal, ri);
  }

  return BRDF::Sample{outgoing, glm::dvec3(1, 1, 1)};
}