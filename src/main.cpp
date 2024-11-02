
#include "camera.h"
#include "geometry.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include <chrono>
#include <memory>
#include <ratio>
#include <string>

std::unique_ptr<Scene> test_scene()
{
  auto scene = std::make_unique<Scene>();

  auto red = std::make_shared<Material>(glm::dvec3(0.77, 0.0, 0.0));
  auto blue = std::make_shared<Material>(glm::dvec3(0.0, 0.0, 0.77));
  auto green = std::make_shared<Material>(glm::dvec3(0.0, 0.77, 0.0));
  auto grey = std::make_shared<Material>(glm::dvec3(0.77));
  auto emissive = std::make_shared<Material>(glm::dvec3(0.99), glm::dvec3(0.99) * 5.0);

#if 1
  {
    // sphere
    Sphere sphere(glm::dvec3(-1.5, .75, -4.0), 1.0);
    Primitive primitive(sphere, red);
    scene->add_primitive(primitive);
  }
#endif
#if 1
  {
    double c = -4;
    double s = 1.0;

    const glm::dvec3 pos(1.5, 0.0, -4.0);

    const glm::dvec3 vert[] = {
        pos + glm::dvec3(+s, 0.5, +s),     // front right
        pos + glm::dvec3(-s, 0.5, +s),     // front left
        pos + glm::dvec3(+s, 0.5, -s),     // back right
        pos + glm::dvec3(-s, 0.5, -s),     // back left
        pos + glm::dvec3(0.0, 2 * s, 0.0)  // top
    };

    // triangle
    Triangle t1(vert[0], vert[4], vert[1]);
    Triangle t2(vert[2], vert[4], vert[0]);
    Triangle t3(vert[3], vert[4], vert[2]);
    Triangle t4(vert[1], vert[4], vert[3]);

    scene->add_primitive(Primitive(t1, red));
    scene->add_primitive(Primitive(t2, red));
    scene->add_primitive(Primitive(t3, red));
    scene->add_primitive(Primitive(t4, red));
  }
#endif
#if 1
  {
    // base
    Primitive primitive(Sphere(glm::dvec3(0.0, -1e5, -4.0), 1e5), grey);
    scene->add_primitive(primitive);
  }
#endif
#if 1
  {
    // light
    Primitive p(Sphere(glm::dvec3(0.0, 5.5, -4.0), 1.5), emissive);
    scene->add_primitive(p);
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
  camera->look_at(glm::dvec3(-1, 1.5, 0), glm::dvec3(0, .5, -4));

  Renderer renderer(camera.get(), scene.get());

  auto start = std::chrono::high_resolution_clock::now();
  renderer.render(samples_per_pixel, max_bounces);
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double, std::milli> duration = end - start;
  double seconds = duration.count() / 1000.0;
  int minutes = seconds / 60;
  seconds -= (minutes * 60);

  fprintf(stderr, "Render time: %dm%.3fs\n", minutes, seconds);

  std::string filename = "render_" + std::to_string(samples_per_pixel) + "_" + std::to_string(max_bounces) + ".png";
  renderer.save_image("result.png");
  renderer.save_image(filename);
  return 0;
}