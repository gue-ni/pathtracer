
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

struct Config {
  bool print_progress;
  int max_bounce;
  int samples_per_pixel;
  int image_width;
  int image_height;

  glm::dvec3 camera_position;
  glm::dvec3 camera_target;
  double camera_fov;
};

namespace glm
{
static void from_json(const json& j, dvec3& v) { v = dvec3{j["x"], j["y"], j["z"]}; }

}  // namespace glm

static void from_json(const json& j, Config& c)
{
  c.print_progress = j["print_progress"];
  c.max_bounce = j["max_bounce"];
  c.samples_per_pixel = j["samples_per_pixel"];
  c.image_width = j["image_width"];
  c.image_height = j["image_height"];

  c.camera_position = j["camera_position"];
  c.camera_target = j["camera_target"];
  c.camera_fov = j["camera_fov"];
}

std::tuple<std::unique_ptr<Scene>, std::unique_ptr<Camera>> test_scene_1()
{
  auto scene = std::make_unique<Scene>();

#if _WIN32
  std::filesystem::path models = "C:/Users/jakob/Documents/Projects/pathtracer/doc/models";
#else
  std::filesystem::path models = "/home/pi/pathtracer/doc/models";
#endif

  auto white = scene->add_material(Material(glm::dvec3(1.0)));
  auto red = scene->add_material(Material(glm::dvec3(.77, 0, 0)));
  auto green = scene->add_material(Material(glm::dvec3(0, 1, 0)));
  auto blue = scene->add_material(Material(glm::dvec3(0, 0, .77)));
  auto emissive = scene->add_material(Material(glm::dvec3(1), glm::dvec3(1) * 10.0));
  auto glass = scene->add_material(Material(Material::TRANSMISSIVE, glm::dvec3(1), glm::dvec3(0)));
  auto pink = scene->add_material(Material(glm::dvec3(1), rgb(252, 15, 192) * 5.0));
  auto mirror = scene->add_material(Material(Material::SPECULAR, glm::dvec3(1), glm::dvec3(0)));

  auto tex = scene->add_material(Material());
  tex->albedo = glm::dvec3(1, 0, 0);
  tex->texture = new Image();
  tex->texture->load(models / std::filesystem::path("uv-test.png"));

#if 1
  auto mesh = scene->load_obj(models / std::filesystem::path("cornell_box.obj"));

  AABB bbox = compute_bounding_volume(mesh.begin(), mesh.end());
  std::cout << "Mesh Size: " << bbox.size() << ", Mesh Center: " << bbox.center() << std::endl;

  scene->set_center(bbox.center());
  scene->set_focus_size(bbox.size());
  scene->add_primitives(mesh.begin(), mesh.end());
#endif
#if 0
  scene->add_primitive(Primitive(Sphere(glm::dvec3(0.0, -1e6, 0.0), 1e6), white));
#endif
#if 0
  // light
  scene->add_primitive(Primitive(Sphere(glm::dvec3(0, 170, 0), 30), emissive));
#endif
#if 1
  scene->add_primitive(Primitive(Sphere(glm::dvec3(+70, 20, 0), 20), mirror));
#endif
#if 1
  scene->add_primitive(Primitive(Sphere(glm::dvec3(-70, 20, 0), 20), tex));
#endif

  std::unique_ptr<Camera> camera = std::make_unique<Camera>(640, 360);
  camera->set_position(glm::dvec3(0, 50, 230));

  std::cout << "Camera Position: " << camera->position() << std::endl;
  std::cout << "Camera Direction: " << camera->direction() << std::endl;

  scene->compute_bvh();

  return std::make_tuple(std::move(scene), std::move(camera));
}

std::tuple<std::unique_ptr<Scene>, std::unique_ptr<Camera>> setup_scene(const Config& config,
                                                                        const std::filesystem::path& model_path)
{
  auto scene = std::make_unique<Scene>();

  if (!model_path.empty()) {
    auto mesh = scene->load_obj(model_path);

    AABB bbox = compute_bounding_volume(mesh.begin(), mesh.end());
    std::cout << "Mesh Size: " << bbox.size() << ", Mesh Center: " << bbox.center() << std::endl;

    scene->set_center(bbox.center());
    scene->set_focus_size(bbox.size());
    scene->add_primitives(mesh.begin(), mesh.end());
  }

  std::unique_ptr<Camera> camera = std::make_unique<Camera>(config.image_width, config.image_height, config.camera_fov);
  camera->look_at(config.camera_position, config.camera_target);

  scene->compute_bvh();

  return std::make_tuple(std::move(scene), std::move(camera));
}

int main(int argc, char** argv)
{
  std::filesystem::path model_path;
  std::filesystem::path config_path;
  std::filesystem::path result_path;

  if (2 <= argc) {
    config_path = std::filesystem::path(argv[1]);
  }
  if (3 <= argc) {
    model_path = std::filesystem::path(argv[2]);
  }

  std::ifstream file(config_path);

  if (!file.is_open()) {
    std::cerr << "Could not open config: " << config_path.string() << std::endl;
    return 1;
  }

  json json_config = json::parse(file);
  Config config;
  from_json(json_config, config);

  const auto now = std::chrono::system_clock::now();
  const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

  std::string filename = "render_" + std::to_string(config.samples_per_pixel) + "s_" +
                         std::to_string(config.max_bounce) + "b_" + std::to_string(timestamp) + ".png";

#if 0
  auto [scene, camera] = test_scene_1();
#else
  auto [scene, camera] = setup_scene(config, model_path);
#endif

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
  std::cout << "Scene Size: " << scene->focus_size() << std::endl;
  std::cout << "Scene Center: " << scene->center() << std::endl;
  std::cout << "Primitive Count: " << scene->primitive_count() << std::endl;

  Renderer renderer(camera.get(), scene.get());

  auto start = std::chrono::high_resolution_clock::now();

  renderer.render(config.samples_per_pixel, config.max_bounce, config.print_progress);

  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;
  double seconds = duration.count() / 1000.0;
  int minutes = seconds / 60;
  seconds -= (minutes * 60);

  print_stats();
  fprintf(stdout, "Render time: %dm%.3fs\n", minutes, seconds);

  renderer.save_image("latest_render.png");
  renderer.save_image(filename.c_str());
  return 0;
}