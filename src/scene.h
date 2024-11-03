
#pragma once

#include <pthread.h>
#include "bvh.h"
#include "geometry.h"
#include "material.h"
#include "ray.h"
#include <glm/glm.hpp>
#include <vector>
#include <atomic>

class Scene
{
 public:
  Scene();
  void compute();
  void add_primitive(const Primitive& p) { primitives.push_back(p); }
  Material* add_material(const Material& m);

  template <typename It>
  void add_primitives(It begin, It end)
  {
    for (auto it = begin; it != end; it++) {
      add_primitive(*it);
    }
  }

  std::optional<Intersection> find_intersection(const Ray&);
  glm::dvec3 background(const Ray&);

 private:
  std::vector<Primitive> primitives;
  std::unique_ptr<BVH> bvh;
  std::vector<Material> materials;
};
