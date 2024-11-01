
#pragma once

#include "geometry.h"
#include "ray.h"
#include <glm/glm.hpp>
#include <vector>

class Scene
{
 public:
  std::vector<Primitive> primitives;

  Scene();
  Intersection find_intersection(const Ray&);
  glm::dvec3 background(const Ray&);
};
