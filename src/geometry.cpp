#include "geometry.h"
#include <optional>
#include "glm/geometric.hpp"

bool ray_vs_sphere(const Ray&, const Sphere&, float& t) { return false; }

bool ray_vs_triangle(const Ray&, const Triangle&, float& t) { return false; }

std::optional<Intersection> Primitive::intersect(const Ray& ray) const
{
  float t;
  switch (type) {
    case SPHERE: {
      if (ray_vs_sphere(ray, sphere, t)) {
        Intersection surface;
        surface.hit = true;
        surface.t = t;
        surface.point = ray.point_at(t);
        surface.normal = glm::normalize(surface.point - sphere.center);
        surface.material = material;
        return surface;
      } else {
        return std::nullopt;
      }
    }
    default:
      return std::nullopt;
  }
}