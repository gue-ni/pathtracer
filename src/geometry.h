#pragma once

#include "aabb.h"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "ray.h"
#include "material.h"
#include "util.h"
#include <optional>
#include <memory>
#include <vector>

struct Intersection {
  bool hit;
  double t;
  glm::dvec3 point;
  glm::dvec3 normal;
  Material* material;
  inline bool is_closer_than(const Intersection& other) const { return t < other.t; }
};

std::optional<Intersection> closest(const std::optional<Intersection> a, const std::optional<Intersection> b);

struct Sphere {
  glm::dvec3 center;
  double radius;
  Sphere(const glm::dvec3& c, double r) : center(c), radius(r) {}
};

bool ray_vs_sphere(const Ray&, const Sphere&, double& t);
bool ray_vs_sphere_v2(const Ray& r, const Sphere& s, const Interval<double>& ti, double& t);

// vertex order is counter-clockwise
struct Triangle {
  glm::dvec3 v0, v1, v2;
  Triangle() {}
  Triangle(const glm::dvec3& _v0, const glm::dvec3& _v1, const glm::dvec3& _v2) : v0(_v0), v1(_v1), v2(_v2) {}
  inline glm::dvec3 normal() const
  {
    auto v0v1 = v1 - v0;
    auto v0v2 = v2 - v0;
    auto normal = glm::cross(v0v1, v0v2);  // N
    return glm::normalize(normal);
  }
  inline glm::dvec3 vertex(size_t i) const
  {
    assert(i < 3);
    if (i == 0) return v0;
    if (i == 1) return v1;
    return v2;
  }
};

bool ray_vs_triangle(const Ray&, const Triangle&, const Interval<double>& ti, double& t);

struct Primitive {
  enum Type : uint8_t { SPHERE, TRIANGLE };
  const Type type;
  const union {
    Sphere sphere;
    Triangle triangle;
  };
  const std::shared_ptr<Material> material;
  const AABB bbox;

  Primitive(const Sphere& s, const std::shared_ptr<Material>& m) : type(SPHERE), sphere(s), material(m), bbox(AABB(s))
  {
  }
  Primitive(const Triangle& t, const std::shared_ptr<Material>& m)
      : type(TRIANGLE), triangle(t), material(m), bbox(AABB(t))
  {
  }
  std::optional<Intersection> intersect(const Ray&) const;
};

std::optional<Intersection> intersect_primitives(const Ray&, const std::vector<Primitive>& primitives);