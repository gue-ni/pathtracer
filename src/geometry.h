#pragma once

#include "aabb.h"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "ray.h"
#include "material.h"
#include "util.h"
#include <filesystem>
#include <optional>
#include <memory>
#include <vector>

struct Intersection {
  double t;
  glm::dvec3 point;
  glm::dvec3 normal;
  glm::dvec2 uv;
  Material* material;
  bool inside;
  inline bool is_closer_than(const Intersection& other) const { return t < other.t; }
  glm::dvec3 albedo() const;
};

std::optional<Intersection> closest(const std::optional<Intersection>& a, const std::optional<Intersection>& b);

struct Sphere {
  glm::dvec3 center;
  double radius;
  Sphere() : center(0, 0, 0), radius(1) {}
  Sphere(const glm::dvec3& c, double r) : center(c), radius(r) {}
  glm::dvec2 texcoord(const glm::dvec3& point_on_sphere) const;
  bool intersect(const Ray& r, const Interval<double>& ti, double& t) const;
};

// vertex order is counter-clockwise
struct Triangle {
  glm::dvec3 v0, v1, v2;  // vertex position
  glm::dvec3 n0, n1, n2;  // normal
  glm::dvec2 t0, t1, t2;  // texture coordinate
  Triangle() {}
  glm::dvec2 texcoord(const glm::dvec3& point_on_triangle) const;
  // interpolated normal
  glm::dvec3 normal(const glm::dvec3& point_on_triangle) const;
  // flat normal
  glm::dvec3 normal() const;
  bool intersect(const Ray& r, const Interval<double>& ti, double& t) const;
};

bool ray_vs_sphere(const Ray&, const Sphere&, const Interval<double>& ti, double& t);
bool ray_vs_triangle(const Ray&, const Triangle&, const Interval<double>& ti, double& t);

struct Primitive {
  enum Type : uint8_t { SPHERE, TRIANGLE };
  Type type;
  union {
    Sphere sphere;
    Triangle triangle;
  };
  Material* material;
  AABB bbox;

  Primitive(const Sphere& s, Material* m) : type(SPHERE), sphere(s), material(m), bbox(AABB(s)) {}
  Primitive(const Triangle& t, Material* m) : type(TRIANGLE), triangle(t), material(m), bbox(AABB(t)) {}
  std::optional<Intersection> intersect(const Ray&) const;
  bool is_light() const;
  glm::dvec3 sample_point() const;
  double area() const;
};

void print_stats();
