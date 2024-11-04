
#include "aabb.h"
#include "camera.h"
#include "geometry.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include <chrono>
#include <memory>
#include <ratio>
#include <string>

std::unique_ptr<Scene> test_scene_1()
{
  auto scene = std::make_unique<Scene>();

  auto grey = scene->add_material(Material(glm::dvec3(0.77)));
  auto emissive = scene->add_material(Material(glm::dvec3(0.99), glm::dvec3(0.99) * 5.0));
  auto tangerine_tango = scene->add_material(Material(rgb(217, 65, 37)));
  auto honeysuckle = scene->add_material(Material(rgb(230, 99, 134)));

#if 1
  {
    // sphere
    Sphere sphere(glm::dvec3(-1.5, 1.2, -4.0), 1.0);
    Primitive primitive(sphere, tangerine_tango);
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

    scene->add_primitive(Primitive(t1, honeysuckle));
    scene->add_primitive(Primitive(t2, honeysuckle));
    scene->add_primitive(Primitive(t3, honeysuckle));
    scene->add_primitive(Primitive(t4, honeysuckle));
  }
#endif
#if 0
  {
    // base
    Primitive primitive(Sphere(glm::dvec3(0.0, -1e5, 0.0), 1e5), grey);
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

  scene->compute_bvh();
  return scene;
}

std::unique_ptr<Scene> test_scene_2()
{
  auto scene = std::make_unique<Scene>();

  auto grey = scene->add_material(Material(glm::dvec3(0.77)));
  auto emissive = scene->add_material(Material(glm::dvec3(0.99), glm::dvec3(0.99) * 5.0));

#if 1
  auto mesh = scene->load_obj("/home/pi/pathtracer/doc/models/suzanne.obj");
  scene->add_primitives(mesh.begin(), mesh.end());
#endif
#if 1
  {
    // base
    Primitive primitive(Sphere(glm::dvec3(0.0, -1e5 - 5, 0.0), 1e5), grey);
    scene->add_primitive(primitive);
  }
#endif
#if 1
  {
    // light
    Primitive p(Sphere(glm::dvec3(0.0, 5, 0.0), 1.5), emissive);
    scene->add_primitive(p);
  }
#endif

#if 0
  {
    auto honeysuckle = scene->add_material(Material(rgb(230, 99, 134)));

    double s = 1.0;

    const glm::dvec3 pos(-3, 0, 0);

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

    std::vector<Primitive> triangles;
    triangles.push_back(Primitive(t1, honeysuckle));
    triangles.push_back(Primitive(t2, honeysuckle));
    triangles.push_back(Primitive(t3, honeysuckle));
    triangles.push_back(Primitive(t4, honeysuckle));

    scene->add_primitives(triangles.begin(), triangles.end());

    std::cout << "Triangles: " << triangles.size() << std::endl;
    for (const Primitive& p : triangles) {
      std::cout << "Triangle(" << p.triangle.v0 << ", " << p.triangle.v1 << ", " << p.triangle.v2
                << "), normal=" << p.triangle.normal() << std::endl;
    }
  }
#endif

  scene->compute_bvh();
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

  std::unique_ptr<Scene> scene = test_scene_2();
  std::cout << "Primitive Count: " << scene->primitive_count() << std::endl;

  std::unique_ptr<Camera> camera = std::make_unique<Camera>(640, 360);
  auto center = scene->center();
  std::cout << "Scene center: " << center << std::endl;
  camera->look_at(glm::dvec3(1, 2, 5), glm::dvec3(0, 0, 0));

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

  std::string filename = "render_" + std::to_string(samples_per_pixel) + "s_" + std::to_string(max_bounces) + "b.png";
  renderer.save_image(filename.c_str());
  return 0;
}