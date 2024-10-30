
#include "scene.h"
#include <numbers>
#include "geometry.h"
#include "glm/fwd.hpp"

Scene::Scene() {}

Intersection Scene::find_intersection(const Ray& ray)
{
  Intersection closest;
  closest.hit = false;
  closest.t = 1e9;

  for (const Primitive& primitive : primitives) {
    std::optional<Intersection> result = primitive.intersect(ray);

    if (result.has_value()) {
      if (result.value().t < closest.t) {
        closest = result.value();
      }
    }
  }

  return closest;
}

glm::dvec3 uniform_hemisphere_sampling(const glm::dvec3& normal)
{
  // TODO
  return {};
}

glm::dvec3 Scene::trace_ray(const Ray& ray, int depth)
{
  Intersection surface = find_intersection(ray);

  if (!surface.hit) {
    return glm::dvec3(0.0);
  }

  Material* material = surface.material;
  glm::dvec3 albedo = material->albedo;
  glm::dvec3 emitted = material->radiance;

  if (depth == 0) {
    return emitted;
  }

  Ray scatter;
  scatter.origin = surface.point;
  scatter.direction = uniform_hemisphere_sampling(surface.normal);

  double pdf = 1.0 / (2.0 * std::numbers::pi);
  double cos_theta = glm::dot(surface.normal, scatter.direction);

  return emitted + (albedo / std::numbers::pi) * cos_theta / pdf;
}