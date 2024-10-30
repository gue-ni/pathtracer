#pragma once

#include <filesystem>
#include <memory>

#include "camera.h"
#include "scene.h"

class Renderer
{
 public:
  Renderer(Camera *camera, Scene *scene);
  ~Renderer();
  void render();
  void save_image(const std::filesystem::path &path);

 private:
  int m_width, m_height;
  glm::dvec3 *m_buffer;
  Camera *m_camera;
  Scene *m_scene;
};