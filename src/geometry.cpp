#include "geometry.h"
#include <cstdio>
#include <optional>
#include <glm/glm.hpp>
#include "glm/fwd.hpp"
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

bool ray_vs_triangle(const Ray& r, const Triangle& tri, const Interval<double>& ti, double& t)
{
  // Compute the plane's normal
  glm::dvec3 v0v1 = tri.v1 - tri.v0;
  glm::dvec3 v0v2 = tri.v2 - tri.v0;

  // No need to normalize
  glm::dvec3 N = glm::cross(v0v1, v0v2);  // N

  // Step 1: Finding P

  // Check if the ray and plane are parallel
  double NdotRayDirection = glm::dot(N, r.direction);
  if (glm::abs(NdotRayDirection) < ti.min)  // Almost 0
    return false;                           // They are parallel, so they don't intersect!

  // Compute d parameter using equation 2
  float d = -glm::dot(N, tri.v0);

  // Compute t (equation 3)
  t = -(glm::dot(N, r.origin) + d) / NdotRayDirection;

  // Check if the triangle is behind the ray
  if (t < ti.min) return false;  // The triangle is behind

  // Compute the intersection point using equation 1
  glm::dvec3 P = r.point_at(t);

  // Step 2: Inside-Outside Test
  glm::dvec3 Ne;  // Vector perpendicular to triangle's plane

  // Test sidedness of P w.r.t. edge v0v1
  glm::dvec3 v0p = P - tri.v0;
  Ne = glm::cross(v0v1, v0p);
  if (glm::dot(N, Ne) < 0) return false;  // P is on the right side

  // Test sidedness of P w.r.t. edge v2v1
  glm::dvec3 v2v1 = tri.v2 - tri.v1;
  glm::dvec3 v1p = P - tri.v1;
  Ne = glm::cross(v2v1, v1p);
  if (glm::dot(N, Ne) < 0) return false;  // P is on the right side

  // Test sidedness of P w.r.t. edge v2v0
  glm::dvec3 v2v0 = tri.v0 - tri.v2;
  glm::dvec3 v2p = P - tri.v2;
  Ne = glm::cross(v2v0, v2p);
  if (glm::dot(N, Ne) < 0) return false;  // P is on the right side

  return true;  // The ray hits the triangle
}

std::optional<Intersection> Primitive::intersect(const Ray& ray) const
{
  double t;
  Interval<double> ti(0.001, 1e9);
  switch (type) {
    case SPHERE: {
      if (ray_vs_sphere_v2(ray, sphere, ti, t)) {
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
    case TRIANGLE: {
      if (ray_vs_triangle(ray, triangle, ti, t)) {
        Intersection surface;
        surface.hit = true;
        surface.t = t;
        surface.point = ray.point_at(t);
        surface.normal = triangle.normal();
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
