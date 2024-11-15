
#include "aabb.h"
#include "geometry.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include "image.h"
#include <chrono>
#include <memory>
#include <ratio>
#include <string>
#include <fstream>
#include <tuple>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct SimpleSphere : public Sphere {
  Material::Type type;
  glm::dvec3 albedo;
  glm::dvec3 emissive;
  std::string texture;
  double metallic;
  double roughness;
};

struct Config {
  bool print_progress;
  int max_bounce;
  int samples_per_pixel;
  int batch_size;
  int image_width;
  int image_height;

  glm::dvec3 camera_position;
  glm::dvec3 camera_target;
  double camera_fov;
  double camera_aperture;
  double camera_focus_distance;

  std::vector<std::string> models;
  std::vector<SimpleSphere> spheres;

  std::string environment_texture;
};

namespace glm
{
static void from_json(const json& j, dvec3& v) { v = dvec3{j["x"], j["y"], j["z"]}; }

}  // namespace glm

static bool contains_key(const json& j, const std::string& key) { return j.contains(key); }

template <typename T>
static T get_or_else(const json& j, const std::string& key, T default_value)
{
  if (contains_key(j, key)) {
    return j[key];
  } else {
    return default_value;
  }
}

static void from_json(const json& j, SimpleSphere& s)
{
  s.center = get_or_else(j, "center", glm::dvec3(0.0));
  s.radius = get_or_else(j, "radius", 1.0);
  s.albedo = get_or_else(j, "albedo", glm::dvec3(1.0));
  s.emissive = get_or_else(j, "emissive", glm::dvec3(0.0));
  s.texture = get_or_else(j, "texture", std::string());
  s.metallic = get_or_else(j, "metallic", 0.5);
  s.roughness = get_or_else(j, "roughness", 0.5);
  auto type = get_or_else(j, "type", std::string("DIFFUSE"));

  if (type == "SPECULAR") {
    s.type = Material::SPECULAR;
  } else if (type == "TRANSMISSIVE") {
    s.type = Material::TRANSMISSIVE;
  } else {
    s.type = Material::DIFFUSE;
  }
}

static void from_json(const json& j, Config& c)
{
  c.print_progress = get_or_else(j, "print_progress", false);
  c.image_width = get_or_else(j, "image_width", 640);
  c.image_height = get_or_else(j, "image_height", 480);

  c.camera_position = get_or_else(j, "camera_position", glm::dvec3(0.0, 0.0, 10.0));
  c.camera_target = get_or_else(j, "camera_target", glm::dvec3(0.0, 0.0, 0.0));
  c.camera_fov = get_or_else(j, "camera_fov", 45);
  c.camera_aperture = get_or_else(j, "camera_aperture", 0);
  c.camera_focus_distance = get_or_else(j, "camera_focus_distance", 0);

  c.environment_texture = get_or_else(j, "environment_texture", std::string());

  if (contains_key(j, "models")) {
    c.models = j["models"].get<std::vector<std::string>>();
  }

  if (contains_key(j, "spheres")) {
    c.spheres = j["spheres"].get<std::vector<SimpleSphere>>();
  }
}

std::tuple<std::unique_ptr<Scene>, std::unique_ptr<Camera>> setup_scene(const Config& config)
{
  auto scene = std::make_unique<Scene>();

  for (const std::string path : config.models) {
    auto mesh = scene->load_obj(path);
    AABB bbox = compute_bounding_volume(mesh.begin(), mesh.end());
    std::cout << "Mesh Size: " << bbox.size() << ", Mesh Center: " << bbox.center() << std::endl;
    scene->add_primitives(mesh.begin(), mesh.end());
  }

  for (const auto& s : config.spheres) {
    Material* material = scene->add_material(Material());

    material->type = s.type;
    material->albedo = s.albedo;
    material->emission = s.emissive;
    material->roughness = s.roughness;
    material->metallic = s.metallic;

    if (!s.texture.empty()) {
      material->texture = new Image();  // TODO: this is never deallocated
      if (material->texture->load(s.texture)) {
        std::cout << "Loaded texture " << s.texture << std::endl;
      } else {
        std::cerr << "Failed to load texture " << s.texture << std::endl;
      }
    }
    Primitive p(Sphere(s.center, s.radius), material);
    scene->add_primitive(p);
  }

  if (!config.environment_texture.empty()) {
    Image* image = new Image();
    if (image->load(config.environment_texture)) {
      std::cout << "Loaded environment texture " << config.environment_texture << std::endl;
      scene->set_envmap(image);
    } else {
      std::cerr << "Failed to load environment texture " << config.environment_texture << std::endl;
      exit(1);
    }
  }

  std::unique_ptr<Camera> camera = std::make_unique<Camera>(config.image_width, config.image_height, config.camera_fov,
                                                            config.camera_aperture, config.camera_focus_distance);
  camera->look_at(config.camera_position, config.camera_target);

  std::cout << "Distance camera position to camera target: "
            << glm::distance(config.camera_position, config.camera_target) << std::endl;

  scene->compute_bvh();

  return std::make_tuple(std::move(scene), std::move(camera));
}

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <path to config.json> <output path> <samples> <bounces> <batch_size>"
              << std::endl;
    return 1;
  }

  std::filesystem::path config_path = std::filesystem::path(argv[1]);
  std::filesystem::path result_path = std::filesystem::path(argv[2]);

  std::ifstream file(config_path);

  if (!file.is_open()) {
    std::cerr << "Could not open config: " << config_path.string() << std::endl;
    return 1;
  }

  json json_config = json::parse(file);
  Config config;
  from_json(json_config, config);

  if (4 <= argc) {
    config.samples_per_pixel = std::atoi(argv[3]);
  } else {
    config.samples_per_pixel = 8;
  }

  if (5 <= argc) {
    config.max_bounce = std::atoi(argv[4]);
  } else {
    config.max_bounce = 3;
  }

  if (6 <= argc) {
    config.batch_size = std::atoi(argv[5]);
  } else {
    config.batch_size = 16;
  }

  const auto now = std::chrono::system_clock::now();
  const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

  auto [scene, camera] = setup_scene(config);

  if (!scene) {
    std::cerr << "Failed to setup scene!\n";
    return 1;
  }

  if (!camera) {
    std::cerr << "Failed to setup camera!\n";
    return 1;
  }

  std::cout << "Samples Per Pixel: " << config.samples_per_pixel << std::endl;
  std::cout << "Max Bounce Depth: " << config.max_bounce << std::endl;
  std::cout << "Camera Position: " << camera->position() << std::endl;
  std::cout << "Camera Direction: " << camera->direction() << std::endl;
  std::cout << "Scene Size: " << scene->size() << std::endl;
  std::cout << "Scene Center: " << scene->center() << std::endl;
  std::cout << "Primitive Count: " << scene->primitive_count() << std::endl;

  Renderer renderer(camera.get(), scene.get());

  auto start = std::chrono::high_resolution_clock::now();

  int batch = config.batch_size;

  if (0 < batch) {
    while (renderer.total_samples < config.samples_per_pixel) {
      int todo = config.samples_per_pixel - renderer.total_samples;
      if (batch > todo) batch = todo;
      renderer.render(batch, config.max_bounce, config.print_progress);

      printf("%d/%d samples per pixel\n", renderer.total_samples, config.samples_per_pixel);

      auto path = result_path.parent_path() / std::filesystem::path("latest.png");
      renderer.save_image(path);
    }
  } else {
    renderer.render(config.samples_per_pixel, config.max_bounce, config.print_progress);
  }

  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;
  double seconds = duration.count() / 1000.0;
  int minutes = seconds / 60;
  seconds -= (minutes * 60);

  print_stats();
  printf("%d Samples/Pixel\n", renderer.total_samples);
  fprintf(stdout, "Render time: %dm%.3fs\n", minutes, seconds);

  renderer.save_image(result_path);
  return 0;
}