
#pragma once

#include "bvh.h"
#include "geometry.h"
#include "material.h"
#include "ray.h"
#include "medium.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <atomic>
#include <cstdint>

class Scene
{
 public:
  Scene();
  Scene(std::unique_ptr<Medium> medium);
  void compute_bvh();
  void add_primitive(const Primitive& p);
  void add_primitives(const std::vector<Primitive>::iterator begin, const std::vector<Primitive>::iterator end);
  Material* add_material(const Material& m);
  std::vector<Primitive> load_obj(const std::filesystem::path& filename);
  std::optional<Intersection> find_intersection(const Ray&) const;
  glm::dvec3 sample_background(const Ray&) const;
  int primitive_count();
  glm::dvec3 center() const;
  glm::dvec3 size() const;
  void set_background_texture(std::unique_ptr<Image> texture);
  void set_background_color(const glm::dvec3& color);
  int light_count() const;
  Primitive random_light() const;
  std::vector<Primitive> lights() const;
  const Medium* medium() const;

 private:
  uint32_t m_count;
  std::vector<Primitive> m_primitives;
  std::vector<Primitive> m_lights;
  std::unique_ptr<BVH> m_bvh;
  size_t m_material_count = 0;
  std::array<Material, 256> m_materials;
  std::unique_ptr<Image> m_background_texture;
  glm::dvec3 m_background_color;
  std::unique_ptr<Medium> m_medium;
};
