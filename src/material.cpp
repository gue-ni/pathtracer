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
  double alpha = roughness * roughness;
  double alpha2 = alpha * alpha;
  double NoH2 = NoH * NoH;
  double b = (NoH2 * (alpha2 - 1.0) + 1.0);
  return alpha2 * (1 / pi) / (b * b);
}

static double pdf_GGX(double theta, double roughness)
{
  double alpha = roughness * roughness;
  double alpha2 = alpha * alpha;
  double cos_theta = std::cos(theta);
  // double b = (alpha2 - 1.0) * (cos_theta * cos_theta) + 1.0;
  // return (alpha2 * cos_theta) / (pi * (b * b));

  double num = (alpha2 * cos_theta);
  double a = ((alpha2 - 1.0) * (cos_theta * cos_theta) + 1.0);
  double denom = pi * (a * a);
  return num / denom;
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

//====================================================================
// non height-correlated masking-shadowing function is described here:
static double SmithGGXMaskingShadowing(double NoL, double NoV, double a2)
{
  double denomA = NoV * glm::sqrt(a2 + (1.0 - a2) * NoL * NoL);
  double denomB = NoL * glm::sqrt(a2 + (1.0 - a2) * NoV * NoV);
  return 2.0 * NoL * NoV / (denomA + denomB);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BRDF::BRDF(Intersection* s) : surface(s) {}

// L = Le + (1/N) * ∑ (Li * brdf * cos(theta) * 1/pdf())
BRDF::Sample BRDF::sample(const Ray& incoming)
{
  switch (surface->material->type) {
    case Material::SPECULAR:
#if 1
      return sample_specular(incoming);
#else
      return sample_microfacet(incoming);
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

  double a = roughness;
  double a2 = a * a;

  double phi = 2.0 * pi * e1;
  double theta = std::acos(std::sqrt((1.0 - e0) / ((a2 - 1.0) * e0 + 1.0)));

#if 1
  // why is this so slow?
  // this causes much more bounces
  glm::dvec3 H = vector_from_spherical(theta, phi);
  // glm::dvec3 L = glm::reflect(-V, H);
  glm::dvec3 L = 2.0 * glm::dot(V, H) * H - V;

#else
  glm::dvec3 L = vector_from_spherical(theta, phi);
  glm::dvec3 H = glm::normalize(V + L);
#endif

  double VoH = abs_dot(V, H);
  // since we are in tangent space, dot product with the normal
  // is just the y axis
  double NoL = L.y;
  double NoV = V.y;
  double NoH = H.y;

  glm::dvec3 f0 = glm::mix(glm::dvec3(0.04), surface->albedo(), metallic);
  glm::dvec3 F = SchlickFresnel(f0, VoH);
  double G = SmithGGXMaskingShadowing(NoL, NoV, a2);

  double weight = VoH / (NoV * NoH);
  glm::dvec3 value;

  // glm::dvec3 value = F * G * weight;
  value = F * weight;

  // L = vector_from_spherical(std::acos(std::sqrt(e1)), 2.0 * pi * e0);
  // value = microfacet_brdf(L, V, glm::dvec3(0,1,0), surface->albedo(), 0, metallic, roughness);
  // value /= (L.y / pi);

  Ray outgoing(surface->point, transform * L);
  assert(SameHemisphere(outgoing.direction, surface->normal));
  return BRDF::Sample{outgoing, value};
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

  Ray outgoing(surface->point, glm::refract(incoming.direction, surface->normal, ri));
  return BRDF::Sample{outgoing, glm::dvec3(1, 1, 1)};
}