#include "renderer.h"
#include <memory>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Renderer::Renderer(Camera *camera, Scene *scene) : m_camera(camera), m_scene(scene)
{
  m_buffer = (glm::dvec3 *)malloc(m_camera->width() * m_camera->height() * sizeof(glm::dvec3));
}

Renderer::~Renderer()
{
  if (m_buffer != nullptr) {
    free(m_buffer);
  }
}

void Renderer::render()
{
  int samples = 5;
  double sample_weight = 1.0 / double(samples);

#pragma omp parallel for schedule(dynamic, 1)
  for (int y = 0; y < m_camera->height(); y++) {
    printf("Progress: %.2f%%\n", (double(y) / double(m_camera->height())) * 100.0);

    for (int x = 0; x < m_camera->width(); x++) {
      glm::dvec3 color(0.0, 0.0, 0.0);

      for (int s = 0; s < samples; s++) {
        Ray ray = m_camera->get_ray(x, y);
        color += m_scene->trace_ray(ray);
      }

      m_buffer[y * m_camera->width() + x] = color * sample_weight;
    }
  }
}

void Renderer::save_image(const std::filesystem::path &path)
{
  std::vector<unsigned char> pixels;

  for (int i = 0; i < m_width * m_height; i++) {
    glm::dvec3 color = m_buffer[i];

    pixels.push_back(0xffU);
    pixels.push_back(0x00U);
    pixels.push_back(0xffU);
  }

  if (stbi_write_png(path.c_str(), m_width, m_height, 3, pixels.data(), m_width * 3)) {
    fprintf(stdout, "Image saved successfully!\n");
  } else {
    fprintf(stderr, "Failed to save image!\n");
  }
}