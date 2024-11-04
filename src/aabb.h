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

  inline glm::dvec3 size() const { return max - min; }

  inline glm::dvec3 center() const {
    return min + size() / 2.0;
  }

  inline size_t longest_axis() const
  {
    auto s = size();

    if (s.y >= s.x && s.y >= s.z) {
      return 1;
    }
    if (s.z >= s.x && s.z >= s.y) {
      return 2;
    }

    return 0;
  }
};

AABB merge(const AABB& a, const AABB& b);

AABB compute_bounding_volume(const std::vector<Primitive>::const_iterator& begin,
                             const std::vector<Primitive>::const_iterator& end);

bool ray_vs_aabb(const Ray& r, const AABB& bb, Interval<double> ti);
