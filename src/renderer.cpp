#include "renderer.h"
#include <memory>
#include <iostream>
#include <new>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/io.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PRINT_PROGRESS 0
#define DEBUG_NORMAL   0

using namespace std::numbers;

static double random_double() { return (double)rand() / ((double)RAND_MAX + 1); }

static uint8_t map_pixel(double color) { return static_cast<uint8_t>(glm::clamp(color, 0.0, 1.0) * 255.0); }

static glm::dvec3 random_unit_vector()
{
  // static std::random_device rd;
  // static std::mt19937 gen(rd());
  // std::uniform_real_distribution<double> dist(0.0f, 1.0f);
  double theta = random_double() * 2.0f * pi;
  double phi = std::acos(1.0f - 2.0f * random_double());

  double x = std::sin(phi) * std::cos(theta);
  double y = std::sin(phi) * std::sin(theta);
  double z = std::cos(phi);

  return glm::dvec3(x, y, z);
}

static glm::dvec3 uniform_hemisphere_sampling(const glm::dvec3& normal)
{
  glm::dvec3 unit_vector = random_unit_vector();
  if (glm::dot(unit_vector, normal) > 0.0) {
    return unit_vector;
  } else {
    return -unit_vector;
  }
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
  Intersection surface = m_scene->find_intersection(ray);

  if (!surface.hit) {
    return m_scene->background(ray);
  } else {
#if DEBUG_NORMAL
    return normal_as_color(surface.normal);
#endif
  }

  Material* material = surface.material;
  glm::dvec3 albedo = material->albedo;
  glm::dvec3 emitted = material->radiance;

  if (depth <= 0) {
    return glm::vec3(0);
  }

  Ray scattered;
  scattered.origin = surface.point;
  scattered.direction = uniform_hemisphere_sampling(surface.normal);

  double pdf = 1.0 / (2.0 * pi);
  double cos_theta = glm::max(glm::dot(surface.normal, scattered.direction), 0.0);

  glm::dvec3 indirect = trace_ray(scattered, depth - 1);

  return emitted + (albedo / pi) * indirect * cos_theta / pdf;
}

void Renderer::save_image(const std::filesystem::path& path)
{
  std::vector<unsigned char> pixels;

  for (int i = 0; i < m_camera->width() * m_camera->height(); i++) {
    glm::dvec3 color = m_buffer[i];
    pixels.push_back(map_pixel(color.r));
    pixels.push_back(map_pixel(color.g));
    pixels.push_back(map_pixel(color.b));
  }

  if (stbi_write_png(path.c_str(), m_camera->width(), m_camera->height(), 3, pixels.data(), m_camera->width() * 3)) {
    fprintf(stdout, "Image '%s' saved successfully!\n", path.c_str());
  } else {
    fprintf(stderr, "Failed to save image!\n");
  }
}