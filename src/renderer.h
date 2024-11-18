#pragma once

#include <filesystem>
#include <memory>
#include <iostream>
#include <atomic>

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
  void render(int samples, int max_bounce, bool print_progress = false);
  void render_old(int samples, int max_bounce, bool print_progress = false);
  glm::dvec3 trace_ray(const Ray &ray, int depth, int max_depth);
  glm::dvec3 sample_lights(const glm::dvec3 &point, const BxDF& bsdf);
  void save_image(const std::filesystem::path& path);

  int total_samples = 0;

 private:
  glm::dvec3 *m_buffer;
  Camera *m_camera;
  Scene *m_scene;
};