#include "bvh.h"
#include <optional>
#include "aabb.h"
#include "geometry.h"

bool BVH::Node::is_leaf() const { return left == nullptr && right == nullptr; }

std::optional<Intersection> BVH::Node::intersect(const Ray& ray)
{
  if (!ray_vs_aabb(ray, bbox, Interval<double>(0.01, 1e9))) {
    return std::nullopt;
  }

  if (is_leaf()) {
    return intersect_primitives(ray);
  }

  auto l = left->intersect(ray);
  auto r = right->intersect(ray);
  return closest(l, r);
}

std::optional<Intersection> BVH::Node::intersect_primitives(const Ray& ray)
{
  std::optional<Intersection> closest = std::nullopt;

  for (auto it = begin; it != end; ++it) {
    auto possible_intersection = (*it).intersect(ray);

    if (possible_intersection.has_value()) {
      Intersection its = possible_intersection.value();

      if (closest.has_value()) {
        if (its.is_closer_than(closest.value())) {
          closest = its;
        }
      } else {
        closest = its;
      }
    }
  }
  return closest;
}

BVH::BVH(const std::vector<Primitive>& primitives) : m_primitives(primitives) { m_root = construct(); }

std::optional<Intersection> BVH::traverse(const Ray& ray) { return m_root->intersect(ray); }

std::unique_ptr<BVH::Node> BVH::construct()
{
  // compute bbox
  // split along axis
  // sort primitives along split axis
  // insert primitives into nodes

  return nullptr;
}