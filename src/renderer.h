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
  void render(int samples, int max_bounce, bool print_progress = false);
  void save_image(const std::filesystem::path &path);

  int total_samples = 0;

 private:
  Scene *m_scene;
  Camera *m_camera;
  std::vector<glm::dvec3> m_buffer;

  glm::dvec3 trace_ray(const Ray &ray, int depth, int max_depth, bool perfect_reflection = false);
  glm::dvec3 sample_lights(const glm::dvec3 &point, const BxDF &bsdf, const glm::dvec3 &incoming, uint32_t id);
};