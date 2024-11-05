#include "material.h"
#include <cassert>
#include "geometry.h"
#include "util.h"

glm::dmat3 local_to_world(const glm::dvec3& normal) 
{
  // TODO
  // https://gamedev.stackexchange.com/questions/120352/extract-a-rotation-matrix-given-a-camera-direction-vector-and-a-up-vector-for
  return glm::dmat3();
}

glm::dvec3 vector_from_spherical(double pitch, double yaw) 
{
  // TODO
  return glm::dvec3();
}

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
  glm::dvec3 brdf_value = surface->material->albedo / pi;
  return BRDF::Sample{scattered, brdf_value * cos_theta / pdf};
}

BRDF::Sample BRDF::sample_specular(const Ray& incoming)
{
  Ray reflected = Ray(surface->point, glm::reflect(incoming.direction, surface->normal));
  return BRDF::Sample{reflected, surface->material->albedo};
}

BRDF::Sample BRDF::sample_transmissive(const Ray& incoming)
{
  double refraction_index = surface->material->refraction_index;
  double ri = surface->inside ? (1.0 / refraction_index) : refraction_index;

  Ray refracted;
  refracted.origin = surface->point;
  refracted.direction = glm::refract(incoming.direction, surface->normal, ri);

  return BRDF::Sample{refracted, glm::dvec3(1, 1, 1)};
}