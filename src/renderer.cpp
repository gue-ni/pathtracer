#include "renderer.h"
#include <memory>
#include <iostream>
#include <new>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/io.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define PRINT_PROGRESS 0

Renderer::Renderer(Camera *camera, Scene *scene) : m_camera(camera), m_scene(scene)
{
  m_buffer = new glm::dvec3[m_camera->width() * m_camera->height()];
  std::cout << "global up: " << m_camera->m_global_up << std::endl;
  std::cout << "forward:   " << m_camera->m_forward << std::endl;
  std::cout << "up:        " << m_camera->m_up << std::endl;
  std::cout << "right:     " << m_camera->m_right << std::endl;
}

Renderer::~Renderer()
{
  if (m_buffer != nullptr) delete[] m_buffer;
}

void Renderer::render(int samples, int max_depth)
{
  double sample_weight = 1.0 / double(samples);

  for (int y = 0; y < m_camera->height(); y++) {
#if PRINT_PROGRESS
    printf("Progress: %.2f%%\n", (double(y) / double(m_camera->height())) * 100.0);
#endif

    for (int x = 0; x < m_camera->width(); x++) {
      glm::dvec3 color(0.0);

      for (int s = 0; s < samples; s++) {
        Ray ray = m_camera->get_ray(x, y);
        color += m_scene->trace_ray(ray, max_depth);
      }

      m_buffer[y * m_camera->width() + x] = color * sample_weight;
    }
  }
}

uint8_t map_pixel(double color) { return static_cast<uint8_t>(glm::clamp(color, 0.0, 1.0) * 255.0); }

void Renderer::save_image(const std::filesystem::path &path)
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