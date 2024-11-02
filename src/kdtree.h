#pragma once

#include <optional>
#include <vector>
#include "geometry.h"

struct KdNode {
};

class KdTree
{
 public:
  std::optional<Intersection> find_intersection(const Ray&);
  void add_primitive(const Primitive&);

 private:
  std::vector<Primitive> m_primitives;
};
