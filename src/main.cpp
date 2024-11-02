
#include "camera.h"
#include "geometry.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include <memory>
#include <string>

std::unique_ptr<Scene> test_scene()
{
  auto scene = std::make_unique<Scene>();
#if 1
  {
    // sphere
    Primitive primitive;
    primitive.type = Primitive::SPHERE;
    primitive.sphere.center = glm::dvec3(0.0, .75, -4.0);
    primitive.sphere.radius = 1.0;
    primitive.material = std::make_shared<Material>(glm::dvec3(0.77, 0.0, 0.0));
    scene->primitives.push_back(primitive);
  }
#endif
#if 1
  {
    // base
    Primitive primitive;
    primitive.type = Primitive::SPHERE;
    primitive.sphere.center = glm::dvec3(0.0, -5000.0, -4.0);
    primitive.sphere.radius = 5000.0;
    primitive.material = std::make_shared<Material>(glm::dvec3(0.77));
    scene->primitives.push_back(primitive);
  }
#endif
#if 1
  {
    // light
    Primitive primitive;
    primitive.type = Primitive::SPHERE;
    primitive.sphere.center = glm::dvec3(2.0, 3.0, -4.0);
    primitive.sphere.radius = 0.5;
    primitive.material = std::make_shared<Material>(glm::dvec3(0.99), glm::dvec3(0.99) * 5.0);
    scene->primitives.push_back(primitive);
  }
#endif
  return scene;
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

  std::unique_ptr<Scene> scene = test_scene();

  std::unique_ptr<Camera> camera = std::make_unique<Camera>(640, 360);
  camera->set_forward(glm::dvec3(0, 0, -1));
  camera->set_position(glm::dvec3(0, 1, 0));

  Renderer renderer(camera.get(), scene.get());
  renderer.render(samples_per_pixel, max_bounces);

  std::string filename = "render_" + std::to_string(samples_per_pixel) + "_" + std::to_string(max_bounces) + ".png";
  renderer.save_image("result.png");
  renderer.save_image(filename);
  return 0;
}