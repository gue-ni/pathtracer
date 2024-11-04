
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
  glm::dvec3 center() const;
  glm::dvec3 focus_size() const { return m_focus_size; }
  void set_center(const glm::dvec3& c) { m_center = c; }
  void set_focus_size(const glm::dvec3& s) { m_focus_size = s; }
  size_t primitive_count() const { return primitives.size(); }

 private:
  glm::dvec3 m_center = glm::dvec3();
  glm::dvec3 m_focus_size = glm::dvec3(5);
  std::vector<Primitive> primitives;
  std::unique_ptr<BVH> bvh;
  size_t material_count = 0;
  std::array<Material, 256> materials;
};
