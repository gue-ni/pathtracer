
#include "scene.h"
#include <iostream>
#include "aabb.h"
#include "geometry.h"
#include <glm/glm.hpp>
#include "texture.h"
#include "tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

Scene::Scene() : bvh(nullptr) {}

std::optional<Intersection> Scene::find_intersection(const Ray& ray)
{
#define ENABLE_BVH 1
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
#if 0
  // sky
  double a = 0.5 * (r.direction.y + 1.0);
  return (1.0 - a) * glm::dvec3(1.0, 1.0, 1.0) + a * glm::dvec3(0.5, 0.7, 1.0);
#else
  return glm::dvec3(1);
#endif
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

glm::dvec3 Scene::center() const { return m_center; }

struct Vertex {
  glm::dvec3 pos;
  glm::dvec2 uv;
  int material_id;
};

std::vector<Primitive> Scene::load_obj(const std::filesystem::path& filename)
{
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = filename.parent_path().string();  // Path to look for .mtl files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(filename.string(), reader_config)) {
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
  const std::vector<tinyobj::material_t>& mtls = reader.GetMaterials();

  Material* default_material = add_material(Material(glm::dvec3(1, 1, 0)));

  // Print out vertices, shapes, and materials info
  std::cout << "# of vertices: " << (attrib.vertices.size() / 3) << std::endl;
  std::cout << "# of shapes: " << shapes.size() << std::endl;
  std::cout << "# of materials: " << mtls.size() << std::endl;

  auto offset = material_count;

#if 1
  for (const tinyobj::material_t& m : mtls) {
    Material material;
    material.albedo = glm::dvec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
    material.emittance = glm::dvec3(m.emission[0], m.emission[1], m.emission[2]);

    if (!m.diffuse_texname.empty()) {
      auto diffuse_texname = reader_config.mtl_search_path / std::filesystem::path(m.diffuse_texname);

      Texture2D* texture = new Texture2D();
      if (!texture->load(diffuse_texname)) {
        std::cerr << "Failed to load " << diffuse_texname << std::endl;
      } else {
        std::cout << "Loaded texture " << diffuse_texname << std::endl;
        std::cout << texture->width() << ", " << texture->height() << ", " << texture->channels() << std::endl;
      }

      //material.albedo = glm::dvec3(0,1,1);
      material.texture = texture;
    }

    (void)add_material(material);
  }
#endif

  std::vector<Vertex> vertices;

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // per-face material
      int material_id = shapes[s].mesh.material_ids[f];

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        Vertex vertex;

        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        vertex.pos = {vx, vy, vz};

        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
          tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
          vertex.uv = {tx, ty};
          std::cout << vertex.uv << std::endl;
        }

        vertex.material_id = material_id;
        vertices.push_back(vertex);
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
    auto t0 = vertices[i * 3 + 0].uv;
    auto t1 = vertices[i * 3 + 1].uv;
    auto t2 = vertices[i * 3 + 2].uv;


    Triangle tri(v0, v1, v2);

    if (mtls.empty()) {
      triangles.push_back(Primitive(tri, default_material));
    } else {
      int id = offset + vertices[i * 3].material_id;
      Material* m = &this->materials[id];
      triangles.push_back(Primitive(tri, m));
    }
  }

#if 0
  std::cout << "Triangles: " << triangles.size() << std::endl;
  for (const Primitive& p : triangles) {
    std::cout << "Triangle(" << p.triangle.v0 << ", " << p.triangle.v1 << ", " << p.triangle.v2
              << "), normal=" << p.triangle.normal() << ", albedo=" << p.material->albedo << std::endl;
  }
#endif

  return triangles;
}
