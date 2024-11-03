#include "bvh.h"
#include <memory>
#include <optional>
#include <algorithm>
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

BVH::BVH(const std::vector<Primitive>& primitives) : m_primitives(primitives)
{
  m_root = construct(m_primitives.begin(), m_primitives.end());
}

std::optional<Intersection> BVH::traverse(const Ray& ray) { return m_root->intersect(ray); }

std::unique_ptr<BVH::Node> BVH::construct(const std::vector<Primitive>::iterator& begin,
                                          const std::vector<Primitive>::iterator& end)
{
  auto node = std::make_unique<BVH::Node>();

  // compute bbox
  node->bbox = compute_bounding_volume(begin, end);

  // split along axis
  size_t split_axis = node->bbox.longest_axis();

  size_t count = end - begin;

  if (count > split_threshold) {
    // sort primitives along split axis
    auto heuristic = [split_axis](Primitive a, Primitive b) { return a.bbox.min[split_axis] < b.bbox.min[split_axis]; };
    std::sort(begin, end, heuristic);

    std::vector<Primitive>::iterator middle = begin + (count / 2);

    node->left = construct(begin, middle);
    node->right = construct(middle + 1, end);
  } else {
    // insert primitives into nodes
    node->begin = begin;
    node->end = end;
  }

  return node;
}