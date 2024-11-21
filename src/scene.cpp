
#include "scene.h"
#include <iostream>
#include "aabb.h"
#include "geometry.h"
#include <glm/glm.hpp>
#include "image.h"
#include "material.h"
#include "tiny_obj_loader.h"
#include "util.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

Scene::Scene() : m_bvh(nullptr), m_background_texture(nullptr), m_background_color(-1.0), m_count(0U) {}

std::optional<Intersection> Scene::find_intersection(const Ray& ray)
{
  assert(m_bvh != nullptr);
  return m_bvh->traverse(ray);
}

static glm::dvec3 sky_gradient(const glm::dvec3& direction)
{
  double a = 0.5 * (direction.y + 1.0);
  return (1.0 - a) * glm::dvec3(1.0, 1.0, 1.0) + a * glm::dvec3(0.5, 0.7, 1.0);
}

glm::dvec3 Scene::background(const Ray& r)
{
  if (m_background_texture) {
    auto color = m_background_texture->sample(equirectangular(r.direction));
    return reverse_gamma_correction(color);
  } else {
    if (glm::all(glm::greaterThanEqual(m_background_color, glm::dvec3(0.0)))) {
      return m_background_color;
    } else {
      return sky_gradient(r.direction);
    }
  }
}

Material* Scene::add_material(const Material& m)
{
  assert(m_material_count < m_materials.size());
  m_materials[m_material_count] = m;
  return &m_materials[m_material_count++];
}

void Scene::add_primitive(const Primitive& p)
{
  Primitive p_new = p;
  p_new.id = m_count++;
  if (p_new.is_light()) m_lights.push_back(p_new);
  m_primitives.push_back(p_new);
}

Primitive Scene::random_light()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(0, int(m_lights.size() - 1));
  int random_index = distr(gen);
  return m_lights[random_index];
}

void Scene::add_primitives(const std::vector<Primitive>::iterator begin, const std::vector<Primitive>::iterator end)
{
  for (auto it = begin; it != end; it++) add_primitive(*it);
}

void Scene::compute_bvh() { m_bvh = std::make_unique<BVH>(m_primitives); }

glm::dvec3 Scene::center() const { return m_bvh->root()->bbox.center(); }

glm::dvec3 Scene::size() const { return m_bvh->root()->bbox.size(); }

struct Vertex {
  glm::dvec3 pos{};
  glm::dvec3 normal{};
  glm::dvec2 uv{};
  int material_id = -1;
};

std::vector<Primitive> Scene::load_obj(const std::filesystem::path& filename)
{
  std::cout << __FUNCTION__ << " Filename: " << filename.string() << std::endl;
  tinyobj::ObjReaderConfig reader_config;

  reader_config.mtl_search_path = filename.parent_path().string();  // Path to look for .mtl files
  std::cout << __FUNCTION__ << " mtl search path: " << reader_config.mtl_search_path << std::endl;

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(filename.string(), reader_config)) {
    if (!reader.Error().empty()) {
      std::cerr << __FUNCTION__ << " " << reader.Error();
    }
    exit(1);
  }

  if (!reader.Warning().empty()) {
    std::cout << __FUNCTION__ << " " << reader.Warning();
  }

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  const std::vector<tinyobj::material_t>& mtls = reader.GetMaterials();

  std::cout << __FUNCTION__ << " Vertices: " << (attrib.vertices.size() / 3) << std::endl;
  std::cout << __FUNCTION__ << " Shapes: " << shapes.size() << std::endl;
  std::cout << __FUNCTION__ << " Materials: " << mtls.size() << std::endl;

  auto offset = m_material_count;

  for (const tinyobj::material_t& m : mtls) {
    Material material;

    switch (m.illum) {
      case 0:
      case 1:
      case 2:
        material.type = Material::DIFFUSE;
        break;
      case 3:
      case 5:
      case 8:
        material.type = Material::SPECULAR;
        break;
      case 4:
      case 6:
      case 7:
      case 9:
        material.type = Material::TRANSMISSIVE;
        break;
      default:
        material.type = Material::DIFFUSE;
        break;
    }

    material.albedo = glm::dvec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
    material.emission = glm::dvec3(m.emission[0], m.emission[1], m.emission[2]);
    material.shininess = m.shininess;
#if 1
    // PBR parameters
    material.roughness = m.roughness;
    material.metallic = m.metallic;
#endif

    if (!m.diffuse_texname.empty()) {
      auto diffuse_texname = reader_config.mtl_search_path / std::filesystem::path(m.diffuse_texname);

      Image* texture = new Image();  // TODO: this is never deallocated
      if (texture->load(diffuse_texname)) {
        std::cout << __FUNCTION__ << " Loaded texture " << diffuse_texname << " (" << texture->width() << ", "
                  << texture->height() << ", " << texture->channels() << ")" << std::endl;
      } else {
        std::cerr << __FUNCTION__ << " Failed to load " << diffuse_texname << std::endl;
      }
      material.texture = texture;
    }

    (void)add_material(material);
  }

  std::vector<Vertex> vertices;

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      int material_id = shapes[s].mesh.material_ids[f];

      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        Vertex vertex;

        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        vertex.pos = {vx, vy, vz};

        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
          vertex.uv = {tx, ty};
        } else {
          vertex.uv = {0, 0};
        }

        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
          vertex.normal = {nx, ny, nz};
        } else {
          vertex.normal = {0, 0, 0};
        }

        vertex.material_id = material_id;
        vertices.push_back(vertex);
      }
      index_offset += fv;
    }
  }

  std::vector<Primitive> triangles;

  Material* default_material = add_material(Material(glm::dvec3(0.5)));

  size_t triangle_count = vertices.size() / 3;
  for (size_t i = 0; i < triangle_count; i++) {
    Triangle tri;
    tri.v0 = vertices[i * 3 + 0].pos;
    tri.v1 = vertices[i * 3 + 1].pos;
    tri.v2 = vertices[i * 3 + 2].pos;
    tri.t0 = vertices[i * 3 + 0].uv;
    tri.t1 = vertices[i * 3 + 1].uv;
    tri.t2 = vertices[i * 3 + 2].uv;
    tri.n0 = vertices[i * 3 + 0].normal;
    tri.n1 = vertices[i * 3 + 1].normal;
    tri.n2 = vertices[i * 3 + 2].normal;

#if 1
    if (mtls.empty()) {
      triangles.push_back(Primitive(tri, default_material));
    } else {
      int id = offset + vertices[i * 3].material_id;
      Material* m = &m_materials[id];
      triangles.push_back(Primitive(tri, m));
    }
#else
    triangles.push_back(Primitive(tri, default_material));
#endif
  }

  std::cout << __FUNCTION__ << " Triangles: " << triangles.size() << std::endl;
  return triangles;
}
