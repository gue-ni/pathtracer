#pragma once

#include <optional>
#include <vector>
#include "aabb.h"
#include "geometry.h"

struct KdNode {
  AABB bb;
  KdNode *left, *right;
  std::optional<Intersection> intersect(const Ray&);
};

class KdTree
{
 public:
  KdTree(const std::vector<Primitive>& objects) : m_primitives(objects) {
    construct();
  }
  std::optional<Intersection> intersect(const Ray&);

 private:
  std::vector<Primitive> m_primitives;
  void construct();
};
