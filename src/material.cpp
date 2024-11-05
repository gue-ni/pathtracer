#include "material.h"
#include <cassert>
#include "geometry.h"
#include "util.h"

glm::dmat3 local_to_world(const glm::dvec3& up)
{
  // TODO
  // https://gamedev.stackexchange.com/questions/120352/extract-a-rotation-matrix-given-a-camera-direction-vector-and-a-up-vector-for
  glm::dvec3 world_up(0, 1, 0);
  glm::dvec3 right = glm::cross(up, world_up);
  glm::dvec3 forward = glm::cross(right, up);
  return glm::dmat3(right, up, forward);
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
  glm::dvec3 reflected = glm::reflect(incoming.direction, surface->normal);
  Ray ray = Ray(surface->point, reflected);
  return BRDF::Sample{ray, surface->material->albedo};
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