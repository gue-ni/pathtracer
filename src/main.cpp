
#include "aabb.h"
#include "geometry.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include <chrono>
#include <memory>
#include <ratio>
#include <filesystem>
#include <string>

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
  tex->texture = new Texture2D();
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
  // base
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
  camera->set_position(glm::dvec3(0, 50, 100));

  std::cout << "Camera Position: " << camera->position() << std::endl;
  std::cout << "Camera Direction: " << camera->direction() << std::endl;

  scene->compute_bvh();

  return std::make_tuple(std::move(scene), std::move(camera));
}

int main(int argc, char** argv)
{
  int samples_per_pixel = 8;
  int max_bounces = 3;

  if (2 <= argc) {
    samples_per_pixel = std::atoi(argv[1]);
  }
  if (3 <= argc) {
    max_bounces = std::atoi(argv[2]);
  }

  fprintf(stdout, "Samples Per Pixel: %d, Max Bounce: %d\n", samples_per_pixel, max_bounces);

  auto [scene, camera] = test_scene_1();
  std::cout << "Primitive Count: " << scene->primitive_count() << std::endl;

  Renderer renderer(camera.get(), scene.get());

  auto start = std::chrono::high_resolution_clock::now();
  renderer.render(samples_per_pixel, max_bounces);
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;
  double seconds = duration.count() / 1000.0;
  int minutes = seconds / 60;
  seconds -= (minutes * 60);

  fprintf(stderr, "Render time: %dm%.3fs\n", minutes, seconds);
  print_stats();

  renderer.save_image("latest_render.png");

  const auto now = std::chrono::system_clock::now();
  const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

  std::string filename = "render_" + std::to_string(samples_per_pixel) + "s_" + std::to_string(max_bounces) + "b_" +
                         std::to_string(timestamp) + ".png";
  renderer.save_image(filename.c_str());
  return 0;
}