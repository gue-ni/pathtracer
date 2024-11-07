#include "texture.h"
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

Texture2D::Texture2D() : m_data(nullptr), m_width(0), m_height(0), m_channels(0) {}

Texture2D::~Texture2D()
{
  if (m_data) {
    stbi_image_free(m_data);
    m_data = nullptr;
  }
}

bool Texture2D::load(const std::filesystem::path& path)
{
  auto tmp = path.string();
  return (m_data = stbi_load(tmp.c_str(), &m_width, &m_height, &m_channels, 3)) != nullptr;
}

glm::u8vec3 Texture2D::pixel(int x, int y) const
{
  assert(0 <= x && x < m_width);
  assert(0 <= y && y < m_height);
  int i = (y * m_width + x) * 3;
  return glm::u8vec3(m_data[i + 0], m_data[i + 1], m_data[i + 2]);
}

glm::dvec3 Texture2D::sample(const glm::dvec2& uv) const { return sample(uv.x, uv.y); }

glm::dvec3 Texture2D::sample(double u, double v) const
{
  if (!((0 <= u && u <= 1) && (0 <= v && v <= 1))) {
    std::cout << u << ", " << v << std::endl;
  }
  assert(0 <= u && u <= 1);
  assert(0 <= v && v <= 1);
#if 0
  return glm::dvec3(u, v, 1);
#else
  // TODO: proper bilinear interpolation
  auto p = pixel(u * (m_width - 1), v * (m_height - 1));
  return rgb(p.r, p.g, p.b);
#endif
}