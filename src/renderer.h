#pragma once

#include <filesystem>
#include <memory>

#include "camera.h"
#include "glm/fwd.hpp"
#include "scene.h"

class Renderer
{
 public:
  Renderer(Camera *camera, Scene *scene);
  ~Renderer();
  void render(int samples, int max_bounce);
  void save_image(const std::filesystem::path &path);

 private:
  glm::dvec3 *m_buffer;
  Camera *m_camera;
  Scene *m_scene;

};