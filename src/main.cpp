
#include "camera.h"
#include "geometry.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include <memory>

std::unique_ptr<Scene> test_scene()
{
  auto scene = std::make_unique<Scene>();

  Material* material = new Material();
  material->albedo = glm::dvec3(1.0, 0.0, 0.0);
  material->radiance = glm::dvec3(0.0, 0.0, 0.0) * 10.0;

  Primitive primitive;
  primitive.type = Primitive::SPHERE;
  primitive.sphere.center = glm::dvec3(0.0, 0.0, 0.0);
  primitive.sphere.radius = 1.0;
  primitive.material = material;

  scene->primitives.push_back(primitive);

  return scene;
}

int main(void)
{
  std::unique_ptr<Scene> scene = test_scene();

  std::unique_ptr<Camera> camera = std::make_unique<Camera>(640, 360);

  Renderer renderer(camera.get(), scene.get());
  renderer.render(5, 3);
  renderer.save_image("test.png");
  return 0;
}