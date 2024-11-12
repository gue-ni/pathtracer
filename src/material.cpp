#include "material.h"
#include <cassert>
#include <cmath>
#include "geometry.h"
#include "util.h"

static glm::dvec3 fresnel_schlick(double cos_theta, const glm::dvec3& F0)
{
  return F0 + (glm::dvec3(1.0) - F0) * glm::pow(1.0 - cos_theta, 5.0);
}

static double D_GGX(double NoH, double roughness)
{
  double alpha = roughness * roughness;
  double alpha2 = alpha * alpha;
  double NoH2 = NoH * NoH;
  double b = (NoH2 * (alpha2 - 1.0) + 1.0);
  return alpha2 * (1 / pi) / (b * b);
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

// Cook-Torrance Microface BRDF
// https://www.youtube.com/watch?v=gya7x9H3mV0
static glm::dvec3 microfacet_brdf(const glm::dvec3& L, const glm::dvec3& V, const glm::dvec3& N,
                                  const glm::dvec3& base_color, double reflectance, double metallic, double roughness

)
{
  // half-way vector
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

  glm::dvec3 F = fresnel_schlick(VoH, f0);
  double D = D_GGX(NoH, roughness);
  double G = G_Smith(NoV, NoL, roughness);

  glm::dvec3 specular = (F * D * G) / (4.0 * glm::max(NoV, 0.001) * glm::max(NoL, 0.001));

  glm::dvec3 rhoD = base_color;
  rhoD *= glm::dvec3(1.0) - F;
  rhoD *= (1.0 - metallic);
  glm::dvec3 diffuse = rhoD * (1.0 / pi);

  return diffuse + specular;
}

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

// https://schuttejoe.github.io/post/ggximportancesamplingpart1/
BRDF::Sample BRDF::sample_microfacet(const Ray& incoming)
{
  double metallic = glm::clamp(surface->material->metallic, 0.0, 0.9);
  double roughness = glm::clamp(surface->material->roughness, 0.01, 1.0);

  glm::dvec3 brdf_value;

  glm::dvec3 wo = -incoming.direction;

  double e0 = random_double(), e1 = random_double();

  double theta = std::acos(std::sqrt((1.0 - e0) / ((a2 - 1.0) * e0 + 1.0)));
  float phi = 2.0 * pi* e1;
  glm::dvec3 wm = local_to_world(surface->normal) * vector_from_spherical(theta, phi);
  glm::dvec3 wi = glm::reflect(wo, wm);

  Ray outgoing(surface->point, wi);
  

  return BRDF::Sample{outgoing, brdf_value};
}

BRDF::Sample BRDF::sample_specular(const Ray& incoming)
{
#if 1
  glm::dvec3 reflected = glm::reflect(incoming.direction, surface->normal);
  Ray outgoing = Ray(surface->point, reflected);

  // TODO: this is probably not linear
  double shininess = surface->material->shininess;
  double max_shininess = 1000;
  if (shininess < max_shininess) {
    double fuzz = 1 - (shininess / max_shininess);
    outgoing.direction += (fuzz * random_unit_vector());
  }

  glm::dvec3 albedo = surface->albedo();
  return BRDF::Sample{outgoing, albedo};
#else

  double metallic = glm::clamp(surface->material->metallic, 0.0, 0.9);
  double roughness = glm::clamp(surface->material->roughness, 0.01, 1.0);
  double reflectance = 0.9;

  // TODO: use better sampling
#if 1
  Ray scattered(surface->point, cosine_weighted_sampling(surface->normal));
  double cos_theta = glm::max(glm::dot(surface->normal, scattered.direction), 0.0);
  double pdf = cos_theta / pi;
#else
  // importance sampling
  double r0 = random_double(), r1 = random_double();
  double phi = 2.0 * pi * r0;
  double alpha = roughness * roughness;
  double theta = std::atan((alpha * std::sqrt(r1)) / (std::sqrt(1.0 - r1)));
  glm::dvec3 direction = local_to_world(surface->normal) * vector_from_spherical(theta, phi);
  Ray scattered(surface->point, direction);
  double cos_theta = glm::max(glm::dot(surface->normal, scattered.direction), 0.0);
  double pdf =
#endif

  glm::dvec3 base_color = surface->albedo();
  glm::dvec3 brdf = microfacet_brdf(scattered.direction, incoming.direction, surface->normal, base_color, reflectance,
                                    metallic, roughness);

  glm::dvec3 value = brdf * cos_theta / pdf;
  return BRDF::Sample{scattered, value};
#endif
}

BRDF::Sample BRDF::sample_transmissive(const Ray& incoming)
{
  double refraction_index = surface->material->refraction_index;
  double ri = surface->inside ? (1.0 / refraction_index) : refraction_index;

  Ray outgoing;
  outgoing.origin = surface->point;
  outgoing.direction = glm::refract(incoming.direction, surface->normal, ri);

  return BRDF::Sample{outgoing, glm::dvec3(1, 1, 1)};
}