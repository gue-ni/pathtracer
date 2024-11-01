#include "geometry.h"
#include <cstdio>
#include <optional>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "glm/exponential.hpp"

bool ray_vs_sphere(const Ray& r, const Sphere& s, double& t)
{
  glm::dvec3 m = r.origin - s.center;
  double b = glm::dot(m, r.direction);
  double c = glm::dot(m, m) - s.radius * s.radius;

  if (c > 0.0 && b > 0.0) return false;
  double discr = b * b - c;

  if (discr < 0.0) return false;

  t = -b - glm::sqrt(discr);
  if (t < 0.0) {
    // actually this should not be happening
    t = 0.0;
  }

  return true;
}



bool ray_vs_sphere_v2(const Ray& r, const Sphere& s, const Interval<double>& ti, double& t)
{
  auto oc = s.center - r.origin;
  auto a = glm::length2(r.direction);
  auto h = glm::dot(r.direction, oc);
  auto c = glm::length2(oc) - s.radius * s.radius;

  auto discriminant = h * h - a * c;
  if (discriminant < 0) return false;

  auto sqrtd = std::sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  auto root = (h - sqrtd) / a;
  if (!ti.surrounds(root)) {
    root = (h + sqrtd) / a;
    if (!ti.surrounds(root)) return false;
  }

  // rec.t = root;
  // rec.p = r.at(rec.t);
  // vec3 outward_normal = (rec.p - center) / radius;
  // rec.set_face_normal(r, outward_normal);
  // rec.mat = mat;

  t = root;
  return true;
}

bool ray_vs_triangle(const Ray&, const Triangle&, double& t) { return false; }

std::optional<Intersection> Primitive::intersect(const Ray& ray) const
{
  double t;
  switch (type) {
    case SPHERE: {
      if (ray_vs_sphere_v2(ray, sphere, Interval<double>(0.001, 1e9), t)) {
        Intersection surface;
        surface.hit = true;
        surface.t = t;
        surface.point = ray.point_at(t);
        surface.normal = (surface.point - sphere.center) / sphere.radius;
        surface.material = material.get();
        return surface;
      } else {
        return std::nullopt;
      }
    }
    default:
      return std::nullopt;
  }
}
