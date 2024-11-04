
#include "scene.h"
#include <memory>
#include <iostream>
#include "aabb.h"
#include "geometry.h"
#include <glm/glm.hpp>
#include <random>
#include "tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

Scene::Scene() : bvh(nullptr) {}

std::optional<Intersection> Scene::find_intersection(const Ray& ray)
{
#define ENABLE_BVH 0
#if ENABLE_BVH == 0
  Intersection closest;
  closest.hit = false;
  closest.t = 1e9;

  for (const Primitive& primitive : primitives) {
    std::optional<Intersection> result = primitive.intersect(ray);

    if (result.has_value()) {
      if (result.value().t < closest.t) {
        closest = result.value();
      }
    }
  }

  return closest.hit ? std::optional<Intersection>(closest) : std::nullopt;
#else
  assert(bvh != nullptr);
  return bvh->traverse(ray);
#endif
}

glm::dvec3 Scene::background(const Ray& r)
{
  // sky
  double a = 0.5 * (r.direction.y + 1.0);
  return (1.0 - a) * glm::dvec3(1.0, 1.0, 1.0) + a * glm::dvec3(0.5, 0.7, 1.0);
}

Material* Scene::add_material(const Material& m)
{
  assert(material_count < materials.size());
  materials[material_count] = m;
  return &materials[material_count++];
}

void Scene::add_primitives(const std::vector<Primitive>::iterator begin, const std::vector<Primitive>::iterator end)
{
  for (auto it = begin; it != end; it++) {
    add_primitive(*it);
  }
}

void Scene::compute_bvh() { bvh = std::make_unique<BVH>(primitives); }

glm::dvec3 Scene::center() const { return bvh->root()->bbox.center(); }

struct Vertex {
  glm::dvec3 pos;
  int material_id;
};

std::vector<Primitive> Scene::load_obj(const std::filesystem::path& filename)
{
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = filename.parent_path();  // Path to look for .mtl files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(filename, reader_config)) {
    if (!reader.Error().empty()) {
      std::cerr << "TinyObjReader: " << reader.Error();
    }
    exit(1);
  }

  if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

  // Print out vertices, shapes, and materials info
  std::cout << "# of vertices: " << (attrib.vertices.size() / 3) << std::endl;
  std::cout << "# of shapes: " << shapes.size() << std::endl;
  std::cout << "# of materials: " << materials.size() << std::endl;

  auto offset = material_count;

#if 0
  for (const tinyobj::material_t& m : materials) {
    Material material;
    material.albedo = glm::dvec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
    material.radiance = glm::dvec3(m.emission[0], m.emission[1], m.emission[2]);
    (void)add_material(material);
  }
#endif

  Material* default_material = add_material(Material(glm::dvec3(1, 0, 1)));

  std::vector<Vertex> vertices;

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // per-face material
      int material_id = shapes[s].mesh.material_ids[f];
      std::cout << "MaterialId = " << material_id << std::endl;

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

        Vertex vert;
        vert.pos = glm::dvec3(vx, vy, vz);
        vert.material_id = material_id;
        vertices.push_back(vert);
      }
      index_offset += fv;
    }
  }

  std::vector<Primitive> triangles;

#if 0
  std::cout << "Vertices: " << vertices.size() << std::endl;
  for (auto& vert : vertices) {
    std::cout << vert.pos << std::endl;
  }
#endif

  size_t triangle_count = vertices.size() / 3;

  for (size_t i = 0; i < triangle_count; i++) {
    auto v0 = vertices[i * 3 + 0].pos;
    auto v1 = vertices[i * 3 + 1].pos;
    auto v2 = vertices[i * 3 + 2].pos;
    Triangle tri(v0, v2, v1);

    int id = offset + vertices[i * 3].material_id;
    Material* m = &this->materials[id];
    triangles.push_back(Primitive(tri, default_material));
  }

#if 1
  std::cout << "Triangles: " << triangles.size() << std::endl;
  for (const Primitive& p : triangles) {
    std::cout << "Triangle(" << p.triangle.v0 << ", " << p.triangle.v1 << ", " << p.triangle.v2
              << "), normal=" << p.triangle.normal() << ", albedo=" << p.material->albedo << std::endl;
  }
#endif

  return triangles;
}
