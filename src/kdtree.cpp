#include "kdtree.h"
#include <cassert>
#include <optional>
#include "aabb.h"
#include "geometry.h"
#include "glm/fwd.hpp"

bool Node::is_leaf() const { return left != nullptr && right != nullptr; }

std::optional<Intersection> Node::intersect(const Ray& ray)
{
  if (is_leaf()) {
    return intersect_primitives(ray, primitives);
  } else {
  }

  return std::nullopt;
}


  KdTree::KdTree(const std::vector<Primitive>& objects) : m_primitives(objects) { m_root = construct(m_primitives); }

std::unique_ptr<Node> KdTree::construct(const std::vector<Primitive>& objects)
{
  assert(!objects.empty());

  AABB bb = objects[0].bbox;
  bb.min = glm::dvec3(+1e9);
  bb.max = glm::dvec3(-1e9);

  for (auto& o : objects)  {
    bb = merge(bb, o.bbox);
  }



  return nullptr;
}

std::optional<Intersection> KdTree::traverse(const Ray& ray) { return m_root->intersect(ray); }