
#include "scene.h"
#include <numbers>
#include "geometry.h"
#include <glm/glm.hpp>
#include <random>

using namespace std::numbers;

static glm::dvec3 random_unit_vector()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());

  std::uniform_real_distribution<double> dist(0.0f, 1.0f);
  double theta = dist(gen) * 2.0f * pi;
  double phi = std::acos(1.0f - 2.0f * dist(gen));

  double x = std::sin(phi) * std::cos(theta);
  double y = std::sin(phi) * std::sin(theta);
  double z = std::cos(phi);

  return glm::dvec3(x, y, z);
}

static glm::dvec3 uniform_hemisphere_sampling(const glm::dvec3& normal)
{
  glm::dvec3 unit_vector = random_unit_vector();
  if (glm::dot(unit_vector, normal) > 0.0) {
    return unit_vector;
  } else {
    return -unit_vector;
  }
}

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

glm::dvec3 Scene::trace_ray(const Ray& ray, int depth)
{
  Intersection surface = find_intersection(ray);

  if (!surface.hit) {
    return glm::dvec3(0.0);
  } else {
    return glm::dvec3(1.0, 0.0, 0.0);
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

  double pdf = 1.0 / (2.0 * pi);
  double cos_theta = glm::dot(surface.normal, scatter.direction);

  return emitted + (albedo / pi) * cos_theta / pdf;
}