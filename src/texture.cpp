#include "texture.h"
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::Texture2D() : m_data(nullptr), m_width(0), m_height(0), m_channels(0) {}

Texture2D::~Texture2D()
{
  if (m_data) {
    stbi_image_free(m_data);
    m_data = nullptr;
  }
}

bool Texture2D::load(const std::string& path)
{
  return (m_data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 3)) != nullptr;
}

glm::u8vec3 Texture2D::pixel(int x, int y) const
{
  int i = (y * m_width + x) * 3;
  return glm::u8vec3(m_data[i + 0], m_data[i + 1], m_data[i + 2]);
}

glm::dvec3 Texture2D::sample(const glm::dvec2& uv) const { return sample(uv.x, uv.y); }

glm::dvec3 Texture2D::sample(double u, double v) const
{
  auto p = pixel(u * (m_width - 1), v * (m_height - 1));
  return rgb(p.r, p.g, p.b);
}
