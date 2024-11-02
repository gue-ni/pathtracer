#pragma once

#include <glm/glm.hpp>
#include "geometry.h"
#include "glm/common.hpp"
#include "ray.h"
#include "util.h"

struct AABB {
  glm::dvec3 min;
  glm::dvec3 max;

  // generate bounding box that contains these two points
  AABB(const glm::dvec3& a, const glm::dvec3& b);
  AABB(const Sphere& s);
  AABB(const Triangle& t);
};

bool ray_vs_aabb(const Ray& r, const AABB& bb, Interval<double> ti);
