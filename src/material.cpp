#include "material.h"
#include <cassert>
#include <tuple>
#include "geometry.h"
#include "util.h"

BRDF::BRDF(Intersection* s) : surface(s) {}

BRDF::Sample BRDF::sample(const Ray& incoming)
{
  switch (surface->material->type) {
    case Material::SPECULAR:
      return sample_specular(incoming);
    case Material::TRANSMISSIVE:
      return sample_transmissive(incoming);
    default:
      return sample_diffuse(incoming);
  }
}

BRDF::Sample BRDF::sample_diffuse(const Ray& incoming)
{
  Ray scattered = Ray(surface->point, cosine_weighted_sampling(surface->normal));
  double cos_theta = glm::max(glm::dot(surface->normal, scattered.direction), 0.0);
  double pdf = cos_theta / pi;

  glm::dvec3 albedo =
      (surface->material->texture) ? surface->material->texture->sample(surface->uv) : surface->material->albedo;

  glm::dvec3 brdf_value = albedo / pi;
  return BRDF::Sample{scattered, brdf_value * cos_theta / pdf};
}

BRDF::Sample BRDF::sample_specular(const Ray& incoming)
{
  Ray reflected = Ray(surface->point, glm::reflect(incoming.direction, surface->normal));
  return BRDF::Sample{reflected, surface->material->albedo};
}

static glm::dvec3 refract(const glm::dvec3& uv, const glm::dvec3& n, double etai_over_etat)
{
  double cos_theta = glm::min(glm::dot(-uv, n), 1.0);
  auto r_out_perp = etai_over_etat * (uv + cos_theta * n);
  auto r_out_parallel = -glm::sqrt(glm::abs(1.0 - glm::dot(r_out_perp, r_out_perp))) * n;
  return r_out_perp + r_out_parallel;
}

BRDF::Sample BRDF::sample_transmissive(const Ray& incoming)
{
  // TODO

  double refraction_index = surface->material->refraction_index;
  double ri = surface->inside ? (1.0 / refraction_index) : refraction_index;

  Ray refracted;
  refracted.origin = surface->point;
  refracted.direction = refract(-incoming.direction, surface->normal, refraction_index);

  return BRDF::Sample{refracted, glm::dvec3(1, 1, 1)};
}