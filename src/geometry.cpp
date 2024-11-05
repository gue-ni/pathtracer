#include "geometry.h"
#include <cstdio>
#include <iostream>
#include <optional>
#include <atomic>
#include <glm/glm.hpp>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "glm/exponential.hpp"

#define ENABLE_COUNTER 1
#if ENABLE_COUNTER
std::atomic<uint64_t> intersection_test_counter = 0;
#endif

bool ray_vs_sphere(const Ray& r, const Sphere& s, const Interval<double>& ti, double& t)
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

  t = root;
  return true;
}

glm::dvec2 Sphere::texcoord(const glm::dvec3& point_on_sphere) const
{
  glm::dvec3 p = glm::normalize(point_on_sphere - center);
  double theta = std::acos(-p.y);
  double phi = std::atan2(-p.z, p.x) + pi;
  double u = phi / (2 * pi);
  double v = theta / pi;
  return {u, v};
}

glm::dvec3 Triangle::barycentric(const glm::dvec3& p) const
{
  auto v0v1 = v1 - v0;
  auto v0v2 = v2 - v0;
  auto v0p = p - v0;

  double d00 = glm::dot(v0v1, v0v1);
  double d01 = glm::dot(v0v1, v0v2);
  double d11 = glm::dot(v0v1, v0v1);
  double d20 = glm::dot(v0p, v0v1);
  double d21 = glm::dot(v0p, v0v2);

  double denom = d00 * d11 - d01 * d01;
  double v = (d11 * d20 - d01 * d21) / denom;
  double w = (d00 * d21 - d01 * d20) / denom;
  double u = 1.0 - v - w;
  return {u, w, v};
}

glm::dvec2 Triangle::texcoord(const glm::dvec3& point_on_triangle) const
{
  // TODO
  glm::dvec3 uvw = barycentric(point_on_triangle);
  return uvw.x * t0 + uvw.y * t1 + uvw.z * t2;
}

glm::dvec3 Triangle::normal() const
{
  auto v0v1 = v1 - v0;
  auto v0v2 = v2 - v0;
  auto normal = glm::cross(v0v1, v0v2);  // N
  return glm::normalize(normal);
}

glm::dvec3 Triangle::vertex(size_t i) const
{
  assert(i < 3);
  if (i == 0) return v0;
  if (i == 1) return v1;
  return v2;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution.html
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
#if ENABLE_COUNTER
  intersection_test_counter++;
#endif
  double t;
  Interval<double> ti(0.001, 1e9);
  switch (type) {
    case SPHERE: {
      if (ray_vs_sphere(ray, sphere, ti, t)) {
        Intersection surface;
        surface.t = t;
        surface.point = ray.point_at(t);
        glm::dvec3 normal = (surface.point - sphere.center) / sphere.radius;
        if (dot(ray.direction, normal) > 0.0) {
          // ray is inside the sphere
          surface.normal = -normal;
          surface.inside = false;
        } else {
          // ray is outside the sphere
          surface.normal = normal;
          surface.inside = true;
        }
        surface.material = material;
        surface.uv = sphere.texcoord(surface.point);
        return surface;
      } else {
        return std::nullopt;
      }
    }
    case TRIANGLE: {
      if (ray_vs_triangle(ray, triangle, ti, t)) {
        Intersection surface;
        surface.t = t;
        surface.point = ray.point_at(t);
        surface.normal = triangle.normal();
        surface.inside = false;
        surface.material = material;
        surface.uv = triangle.texcoord(surface.point);
        return surface;
      } else {
        return std::nullopt;
      }
    }
    default:
      return std::nullopt;
  }
}

std::optional<Intersection> intersect_primitives(const Ray&, const std::vector<Primitive>& primitives)
{
  return std::nullopt;
}

std::optional<Intersection> closest(const std::optional<Intersection>& a, const std::optional<Intersection>& b)
{
  if (a.has_value() && b.has_value()) {
    return (a.value().t < b.value().t) ? a : b;
  }

  if (a.has_value()) {
    return a;
  }

  if (b.has_value()) {
    return b;
  }

  return std::nullopt;
}

void print_stats()
{
#if ENABLE_COUNTER
  std::cout << "Intersection Test Count: " << intersection_test_counter.load() << std::endl;
#endif
}
