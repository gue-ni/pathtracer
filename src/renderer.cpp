#include "renderer.h"
#include "geometry.h"
#include "material.h"
#include <atomic>
#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PRINT_PROGRESS  1
#define DEBUG_NORMAL    0
#define COSINE_WEIGHTED 1

std::atomic<bool> cancel = false;

void handle_sigterm(int signum) {
  cancel = true;
}

static uint8_t map_pixel(double color) { return static_cast<uint8_t>(glm::clamp(color, 0.0, 1.0) * 255.0); }

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
  bool cancel_enabled = omp_get_cancellation();
  if (!cancel_enabled) {
    fprintf(stderr, "omp cancel is not enabled\n");
  }

  #pragma omp parallel
  {
  #pragma omp for
  for (int y = 0; y < m_camera->height(); y++) {

    if (cancel) {
      #pragma omp cancel for
    }

#if PRINT_PROGRESS
    printf("Progress: %.2f%%\n", (double(y) / double(m_camera->height())) * 100.0);
#endif

    for (int x = 0; x < m_camera->width(); x++) {
      glm::dvec3 color(0.0);

      for (int s = 0; s < samples; s++) {
        Ray ray = m_camera->get_ray(x, y);
        color += trace_ray(ray, max_bounce);
      }

      m_buffer[y * m_camera->width() + x] = color /  double(samples);
    }
  }
  }
}

glm::dvec3 Renderer::trace_ray(const Ray& ray, int depth)
{
  auto possible_hit = m_scene->find_intersection(ray);

  if (!possible_hit.has_value()) {
    return m_scene->background(ray);
  }

  Intersection surface = possible_hit.value();

#if DEBUG_NORMAL
  return normal_as_color(surface.normal);
#endif

  Material* material = surface.material;

  if (depth <= 0) {
    return glm::vec3(0);
  }

  BRDF::Sample sample = BRDF(&surface).sample(ray);

  return material->emittance + trace_ray(sample.ray, depth - 1) * sample.value;
}

void Renderer::save_image(const char* path)
{
  std::vector<unsigned char> pixels;

  for (int i = 0; i < m_camera->width() * m_camera->height(); i++) {
    glm::dvec3 color = m_buffer[i];
    pixels.push_back(map_pixel(color.r));
    pixels.push_back(map_pixel(color.g));
    pixels.push_back(map_pixel(color.b));
  }

  if (stbi_write_png(path, m_camera->width(), m_camera->height(), 3, pixels.data(), m_camera->width() * 3)) {
    fprintf(stdout, "Image '%s' saved successfully!\n", path);
  } else {
    fprintf(stderr, "Failed to save image!\n");
  }
}