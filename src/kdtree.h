#pragma once

#include <memory>
#include <optional>
#include <vector>
#include "aabb.h"
#include "geometry.h"

struct Node : public AABB {
  Node *left, *right;
  std::vector<Primitive> primitives;
  bool is_leaf() const;
  std::optional<Intersection> intersect(const Ray&);
};

class KdTree
{
 public:
  KdTree(const std::vector<Primitive>& objects) : m_primitives(objects) { m_root = construct(); }
  std::optional<Intersection> traverse(const Ray&);

 private:
  std::unique_ptr<Node> m_root;
  std::vector<Primitive> m_primitives;

  std::unique_ptr<Node> construct();
};
