#pragma once

#include <filesystem>
#include <memory>
#include <iostream>

#include "camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include <glm/glm.hpp>

#include "scene.h"

class Renderer
{
 public:
  Renderer(Camera *camera, Scene *scene);
  ~Renderer();
  void render(int samples, int max_bounce);
  glm::dvec3 trace_ray(const Ray &ray, int depth);
  void save_image(const char* path);

 private:
  glm::dvec3 *m_buffer;
  Camera *m_camera;
  Scene *m_scene;
};