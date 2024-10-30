
#pragma once

#include "geometry.h"
#include "ray.h"
#include <glm/glm.hpp>
#include <vector>

class Scene
{
 public:
  Scene();
  glm::dvec3 trace_ray(const Ray &ray, int depth);

  std::vector<Primitive> primitives;

  private:
  Intersection find_intersection(const Ray& ray);
};
