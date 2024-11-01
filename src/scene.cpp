
#include "scene.h"
#include <numbers>
#include "geometry.h"
#include <glm/glm.hpp>
#include <random>

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

glm::dvec3 Scene::background(const Ray& r)
{
  double a = 0.5 * (r.direction.y + 1.0);
  return (1.0 - a) * glm::dvec3(1.0, 1.0, 1.0) + a * glm::dvec3(0.5, 0.7, 1.0);
}
