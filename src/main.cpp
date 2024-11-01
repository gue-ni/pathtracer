
#include "camera.h"
#include "geometry.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include <memory>

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
#if 0
  {
    // light
    Primitive primitive;
    primitive.type = Primitive::SPHERE;
    primitive.sphere.center = glm::dvec3(2.0, 10.0, -4.0);
    primitive.sphere.radius = 5.0;
    primitive.material = std::make_shared<Material>(glm::dvec3(0.99), glm::dvec3(0.99) * 1.0);
    scene->primitives.push_back(primitive);
  }
#endif
  return scene;
}

int main(int argc, char** argv)
{
  std::unique_ptr<Scene> scene = test_scene();

  std::unique_ptr<Camera> camera = std::make_unique<Camera>(640, 360);
  camera->set_forward(glm::dvec3(0, 0, -1));
  camera->set_position(glm::dvec3(0, 1, 0));

  Renderer renderer(camera.get(), scene.get());
  renderer.render(64, 3);
  renderer.save_image("result.png");
  return 0;
}