#include "material.h"
#include <cassert>
#include <tuple>
#include "geometry.h"
#include "util.h"

BRDF::BRDF(Intersection* s) : surface(s) {}

std::tuple<Ray, glm::dvec3> BRDF::sample(const Ray& incoming)
{
  switch (surface->material->type) {
    case Material::SPECULAR:
      return sample_specular(incoming);
    default:
      return sample_diffuse(incoming);
  }
}

std::tuple<Ray, glm::dvec3> BRDF::sample_diffuse(const Ray& incoming)
{
  Ray scattered = Ray(surface->point, cosine_weighted_sampling(surface->normal));
  double cos_theta = glm::max(glm::dot(surface->normal, scattered.direction), 0.0);
  double pdf = cos_theta / pi;
  glm::dvec3 brdf_value = surface->material->albedo / pi;
  return std::make_tuple(scattered, brdf_value * cos_theta / pdf);
}

std::tuple<Ray, glm::dvec3> BRDF::sample_specular(const Ray& incoming)
{
  Ray reflected = Ray(surface->point, glm::reflect(incoming.direction, surface->normal));
  return std::make_tuple(reflected, surface->material->albedo);
}