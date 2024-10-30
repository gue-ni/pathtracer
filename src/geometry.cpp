#include "geometry.h"
#include <optional>
#include <glm/glm.hpp>
#include "glm/exponential.hpp"

bool ray_vs_sphere(const Ray& r, const Sphere& s, float& t)
{
  glm::dvec3 m = r.origin - s.center;
  double b = glm::dot(m, r.direction);
  double c = glm::dot(m, m) - s.radius * s.radius;

  if (c > 0.0 && b > 0.0) return false;
  double discr = b * b - c;

  if (discr < 0.0) return false;

  t = -b - glm::sqrt(discr);
  if (t < 0.0) t = 0.0;


  return true;
}

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