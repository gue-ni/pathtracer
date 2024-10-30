
#include "renderer.h"
#include <memory>

int main(void) {

  auto camera = std::make_unique<Camera>(640, 360);
  auto scene = std::make_unique<Scene>();

  Renderer renderer(camera.get(), scene.get());
  renderer.render();
  renderer.save_image("test.png");
  return 0;
}