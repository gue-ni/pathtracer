#include "renderer.h"
#include "geometry.h"
#include "material.h"
#include "util.h"
#include <glm/gtc/type_ptr.hpp>

#define PRINT_PROGRESS  1
#define DEBUG_NORMAL    0
#define COSINE_WEIGHTED 1

static uint8_t map_pixel(double color) { return static_cast<uint8_t>(glm::clamp(color, 0.0, 1.0) * 255.0); }

static glm::u8vec3 map_pixel(const glm::dvec3 color)
{
  return {map_pixel(color.r), map_pixel(color.g), map_pixel(color.b)};
}

static glm::dvec3 normal_as_color(const glm::dvec3& N) { return 0.5 * glm::dvec3(N.x + 1, N.y + 1, N.z + 1); }

Renderer::Renderer(Camera* camera, Scene* scene) : m_camera(camera), m_scene(scene)
{
  m_buffer = new glm::dvec3[m_camera->width() * m_camera->height()];
}

Renderer::~Renderer()
{
  if (m_buffer != nullptr) delete[] m_buffer;
}

void Renderer::render(int samples, int max_bounce)
{
  double sample_weight = 1.0 / double(samples);

#pragma omp parallel for schedule(dynamic, 1)
  for (int y = 0; y < m_camera->height(); y++) {
#if PRINT_PROGRESS
    printf("Progress: %.2f%%\n", (double(y) / double(m_camera->height())) * 100.0);
#endif

    for (int x = 0; x < m_camera->width(); x++) {
      glm::dvec3 color(0.0);

      for (int s = 0; s < samples; s++) {
        Ray ray = m_camera->get_ray(x, y);
        color += trace_ray(ray, max_bounce);
      }

      m_buffer[y * m_camera->width() + x] = color * sample_weight;
    }
  }
}

glm::dvec3 Renderer::trace_ray(const Ray& ray, int depth)
{
  auto possible_hit = m_scene->find_intersection(ray);

  if (!possible_hit.has_value()) {
    return m_scene->background(ray);
  } else {
#if DEBUG_NORMAL
    return normal_as_color(possible_hit.value().normal);
#endif
  }

  Intersection surface = possible_hit.value();
  Material* material = surface.material;

  if (depth <= 0) {
    return glm::vec3(0);
  }

  BRDF::Sample sample = BRDF(&surface).sample(ray);

  return material->emittance + trace_ray(sample.ray, depth - 1) * sample.value;
}

void Renderer::save_image(const char* path)
{
  Image output(m_camera->width(), m_camera->height(), 3);

  for (int y = 0; y < m_camera->height(); y++) {
    for (int x = 0; x < m_camera->width(); x++) {
      glm::dvec3 color = m_buffer[y * m_camera->width() + x];
      color = gamma_correction(color);
      glm::u8vec3 pixel = map_pixel(color);
      output.set_pixel(x, y, glm::value_ptr(pixel));
    }
  }

  output.write(std::filesystem::path(path));
}
