
#pragma once

#include <pthread.h>
#include "geometry.h"
#include "ray.h"
#include <glm/glm.hpp>
#include <vector>

class Scene
{
 public:
  Scene();
  void add_primitive(const Primitive& p) { primitives.push_back(p); }

  template <typename It>
  void add_primitives(It begin, It end)
  {
    for (auto it = begin; it != end; it++) {
      add_primitive(*it);
    }
  }

  Intersection find_intersection(const Ray&);
  glm::dvec3 background(const Ray&);

 private:
  std::vector<Primitive> primitives;
};
