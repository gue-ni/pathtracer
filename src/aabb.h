#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "ray.h"
#include "util.h"

struct Sphere;
struct Triangle;
struct Primitive;

struct AABB {
  glm::dvec3 min;
  glm::dvec3 max;

  // generate bounding box that contains these two points
  AABB() {}
  AABB(const glm::dvec3& a, const glm::dvec3& b);
  AABB(const Sphere&);
  AABB(const Triangle&);
};

AABB merge(const AABB& a, const AABB& b);

AABB bounding_volume(const std::vector<Primitive>& objects);

bool ray_vs_aabb(const Ray& r, const AABB& bb, Interval<double> ti);
