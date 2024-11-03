
#include "scene.h"
#include <memory>
#include <numbers>
#include "aabb.h"
#include "geometry.h"
#include <glm/glm.hpp>
#include <random>

Scene::Scene() : bvh(nullptr) {}

std::optional<Intersection> Scene::find_intersection(const Ray& ray)
{
  #if 0
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
  #else

  assert(bvh != nullptr);

  return bvh->traverse(ray);

  #endif
}

glm::dvec3 Scene::background(const Ray& r)
{
  // sky
  double a = 0.5 * (r.direction.y + 1.0);
  return (1.0 - a) * glm::dvec3(1.0, 1.0, 1.0) + a * glm::dvec3(0.5, 0.7, 1.0);
}

Material* Scene::add_material(const Material& m)
{
  materials.push_back(m);
  return &materials[materials.size() - 1];
}

void Scene::compute() { bvh = std::make_unique<BVH>(primitives); }