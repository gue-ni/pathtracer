#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "aabb.h"
#include "geometry.h"

class BVH
{
 public:
  BVH(const std::vector<Primitive>&);
  std::optional<Intersection> traverse(const Ray&);

 private:
  struct Node {
    std::unique_ptr<Node> left, right;
    AABB bbox;
    std::vector<Primitive>::iterator begin, end;
    bool is_leaf() const;
    std::optional<Intersection> intersect(const Ray&);
    std::optional<Intersection> intersect_primitives(const Ray&);
  };

  const size_t split_threshold = 5;
  std::unique_ptr<Node> m_root;
  std::vector<Primitive> m_primitives;

  std::unique_ptr<Node> construct(const std::vector<Primitive>::iterator& begin, const std::vector<Primitive>::iterator& end);
};