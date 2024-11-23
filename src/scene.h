
#pragma once

#include "bvh.h"
#include "geometry.h"
#include "material.h"
#include "ray.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <atomic>
#include <cstdint>

class Scene
{
 public:
  Scene();
  void compute_bvh();
  void add_primitive(const Primitive& p);
  void add_primitives(const std::vector<Primitive>::iterator begin, const std::vector<Primitive>::iterator end);
  Material* add_material(const Material& m);
  std::vector<Primitive> load_obj(const std::filesystem::path& filename);
  std::optional<Intersection> find_intersection(const Ray&);
  glm::dvec3 background(const Ray&);
  size_t primitive_count() const { return m_primitives.size(); }
  glm::dvec3 center() const;
  glm::dvec3 size() const;
  void set_envmap(Image* envmap) { m_background_texture = envmap; }
  void set_background_color(const glm::dvec3& color) { m_background_color = color; }
  int light_count() const { return m_lights.size(); }
  Primitive random_light();
  std::vector<Primitive> lights() { return m_lights; }

 private:
  uint32_t m_count;
  std::vector<Primitive> m_primitives;
  std::vector<Primitive> m_lights;
  std::unique_ptr<BVH> m_bvh;
  size_t m_material_count = 0;
  std::array<Material, 256> m_materials;
  Image* m_background_texture;
  glm::dvec3 m_background_color;
};
