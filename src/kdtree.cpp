#include "kdtree.h"
#include <optional>
#include "aabb.h"

bool Node::is_leaf() const { return left != nullptr && right != nullptr; }

std::optional<Intersection> Node::intersect(const Ray& ray)
{
  if (is_leaf()) {
    return intersect_primitives(ray, primitives);
  } else {
  }

  return std::nullopt;
}

std::unique_ptr<Node> KdTree::construct() { return nullptr; }

std::optional<Intersection> KdTree::traverse(const Ray& ray) { return m_root->intersect(ray); }