
#pragma once

#include "bvh.h"
#include "geometry.h"
#include "material.h"
#include "ray.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <atomic>

class Scene
{
 public:
  Scene();
  void compute_bvh();
  void add_primitive(const Primitive& p) { primitives.push_back(p); }
  void add_primitives(const std::vector<Primitive>::iterator begin, const std::vector<Primitive>::iterator end);
  Material* add_material(const Material& m);
  std::vector<Primitive> load_obj(const std::filesystem::path& filename);

  std::optional<Intersection> find_intersection(const Ray&);
  glm::dvec3 background(const Ray&);
  size_t primitive_count() const { return primitives.size(); }
  glm::dvec3 center() const;
  glm::dvec3 size() const;

 private:
  std::vector<Primitive> primitives;
  std::unique_ptr<BVH> m_bvh;
  size_t material_count = 0;
  std::array<Material, 256> materials;
};
