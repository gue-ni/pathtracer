#include "geometry.h"
#include <cstdio>
#include <iostream>
#include <optional>
#include <atomic>
#include <glm/glm.hpp>
#include "util.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/io.hpp>
#include <glm/exponential.hpp>

#define ENABLE_COUNTER 1
#if ENABLE_COUNTER
std::atomic<uint64_t> intersection_test_counter = 0;
#endif

glm::dvec3 Intersection::albedo() const
{
  if (material->texture) {
    // image textures are generally sRGB, so we need to convert them to linear space
    return reverse_gamma_correction(material->texture->sample(uv));
  } else {
    return material->albedo;
  }
}

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

bool Sphere::intersect(const Ray& r, const Interval<double>& ti, double& t) const
{
  return ray_vs_sphere(r, *this, ti, t);
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

bool Triangle::intersect(const Ray& r, const Interval<double>& ti, double& t) const
{
  return ray_vs_triangle(r, *this, ti, t);
}

static void barycentric(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c, const glm::dvec3& p, double& u,
                        double& v, double& w)
{
  glm::dvec3 v0 = b - a, v1 = c - a, v2 = p - a;
  double d00 = glm::dot(v0, v0);
  double d01 = glm::dot(v0, v1);
  double d11 = glm::dot(v1, v1);
  double d20 = glm::dot(v2, v0);
  double d21 = glm::dot(v2, v1);
  double inv_denom = 1.0 / (d00 * d11 - d01 * d01);
  v = (d11 * d20 - d01 * d21) * inv_denom;
  w = (d00 * d21 - d01 * d20) * inv_denom;
  u = 1.0 - v - w;
}

glm::dvec2 Triangle::texcoord(const glm::dvec3& point_on_triangle) const
{
  double u, v, w;
  barycentric(v0, v1, v2, point_on_triangle, u, v, w);
  return u * t0 + v * t1 + w * t2;
}

glm::dvec3 Triangle::normal() const
{
  glm::dvec3 v0v1 = v1 - v0;
  glm::dvec3 v0v2 = v2 - v0;
  return glm::normalize(glm::cross(v0v1, v0v2));
}

glm::dvec3 Triangle::normal(const glm::dvec3& point_on_triangle) const
{
  double u, v, w;
  barycentric(v0, v1, v2, point_on_triangle, u, v, w);
  return u * n0 + v * n1 + w * n2;
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
  Interval<double> ti(0.00001, 1e9);
  switch (type) {
    case SPHERE: {
      if (sphere.intersect(ray, ti, t)) {
        Intersection surface;
        surface.id = id;
        surface.t = t;
        surface.point = ray.point_at(t);
        glm::dvec3 normal = (surface.point - sphere.center) / sphere.radius;
        if (glm::dot(ray.direction, normal) > 0.0) {
          // ray is inside the sphere
          surface.normal = -normal;
          surface.inside = false;
        } else {
          // ray is outside the sphere
          surface.normal = normal;
          surface.inside = true;
        }
        surface.material = material;
        if (surface.material->texture) {
          surface.uv = sphere.texcoord(surface.point);
        }
        return surface;
      } else {
        return std::nullopt;
      }
    }
    case TRIANGLE: {
      if (triangle.intersect(ray, ti, t)) {
        Intersection surface;
        surface.id = id;
        surface.t = t;
        surface.point = ray.point_at(t);
        surface.normal = triangle.normal(surface.point);
        surface.inside = false;
        surface.material = material;
        if (surface.material->texture) {
          surface.uv = triangle.texcoord(surface.point);
        }
        return surface;
      } else {
        return std::nullopt;
      }
    }
    default:
      return std::nullopt;
  }
}

bool Primitive::is_light() const { return glm::any(glm::greaterThan(material->emission, glm::dvec3(0.0))); }

// get random point on primitive
glm::dvec3 Primitive::sample_point() const
{
  double r1 = random_double(), r2 = random_double();
  if (type == Type::TRIANGLE) {
    double u = std::sqrt(r1);
    double v = (1.0 - std::sqrt(r1)) * r2;
    double w = 1.0 - u - v;
    return u * triangle.v0 + v * triangle.v1 + w * triangle.v2;
  } else {
    double phi = 2.0 * pi * r1;
    //double theta = std::acos(1.0 - 2.0 * pi);
    double theta = 0;
    return sphere.center + spherical_to_cartesian(theta, phi) * sphere.radius;
  }
}

// get area of primitive
double Primitive::area() const
{
  if (type == Type::TRIANGLE) {
    glm::dvec3 v0v1 = triangle.v1 - triangle.v0;
    glm::dvec3 v0v2 = triangle.v2 - triangle.v0;
    return 0.5 * glm::length(glm::cross(v0v1, v0v2));
  } else {
    return 4.0 * pi * sq(sphere.radius);
  }
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
