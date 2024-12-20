#include "aabb.h"
#include "geometry.h"
#include "glm/fwd.hpp"

AABB::AABB(const glm::dvec3& a, const glm::dvec3& b)
{
  min = glm::min(a, b);
  max = glm::max(a, b);
}

AABB::AABB(const Sphere& s)
{
  min = glm::dvec3(s.center - s.radius);
  max = glm::dvec3(s.center + s.radius);
}

AABB::AABB(const Triangle& t)
{
  min = glm::min(glm::min(t.v0, t.v1), t.v2);
  max = glm::max(glm::max(t.v0, t.v1), t.v2);
}

AABB merge(const AABB& a, const AABB& b)
{
  AABB bb;
  bb.min = glm::min(a.min, b.min);
  bb.max = glm::max(a.max, b.max);
  return bb;
}

AABB compute_bounding_volume(const std::vector<Primitive>::const_iterator& begin,
                     const std::vector<Primitive>::const_iterator& end)
{
  AABB bbox;
  bbox.min = glm::dvec3(+1e9);
  bbox.max = glm::dvec3(-1e9);
  for (auto it = begin; it != end; it++) {
    bbox.min = glm::min(bbox.min, it->bbox.min);
    bbox.max = glm::max(bbox.max, it->bbox.max);
  }

  return bbox;
}

bool ray_vs_aabb(const Ray& r, const AABB& bb, Interval<double> ti)
{
  for (int axis = 0; axis < 3; axis++) {
    Interval<double> ax(bb.min[axis], bb.max[axis]);
    double adinv = 1.0 / r.direction[axis];

    double t0 = (ax.min - r.origin[axis]) * adinv;
    double t1 = (ax.max - r.origin[axis]) * adinv;

    if (t0 < t1) {
      if (t0 > ti.min) ti.min = t0;
      if (t1 < ti.max) ti.max = t1;
    } else {
      if (t1 > ti.min) ti.min = t1;
      if (t0 < ti.max) ti.max = t0;
    }

    if (ti.max <= ti.min) return false;
  }
  return true;
}