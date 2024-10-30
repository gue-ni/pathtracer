
#pragma once

#include "geometry.h"
#include "ray.h"
#include <glm/glm.hpp>
#include <vector>

class Scene
{
 public:
  Scene();
  glm::dvec3 trace_ray(const Ray &ray);

 private:
  std::vector<Primitive> m_primitives;
};
