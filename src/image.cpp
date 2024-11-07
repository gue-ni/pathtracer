#include "image.h"
#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

Image::Image() : m_data(nullptr), m_width(0), m_height(0), m_channels(0) {}

Image::Image(int width, int height, int channels)
    : m_data(nullptr), m_width(width), m_height(height), m_channels(channels)
{
  m_data = (unsigned char*)STBI_MALLOC(width * height * channels);
}

Image::~Image() { free_data(); }

bool Image::load(const std::filesystem::path& path)
{
  free_data();
  auto tmp = path.string();
  return (m_data = stbi_load(tmp.c_str(), &m_width, &m_height, &m_channels, 3)) != nullptr;
}

bool Image::write(const std::filesystem::path& path) const
{
  assert(valid());
  auto tmp = path.string();
  return stbi_write_png(tmp.c_str(), m_width, m_height, m_channels, m_data, m_width * 3);
}

glm::u8vec3 Image::pixel(int x, int y) const
{
  assert(valid());
  assert(0 <= x && x < m_width);
  assert(0 <= y && y < m_height);
  int i = (y * m_width + x) * 3;
  return glm::u8vec3(m_data[i + 0], m_data[i + 1], m_data[i + 2]);
}

void Image::write_pixel(int x, int y, unsigned char* pixel)
{
  assert(valid());
  assert(0 <= x && x < m_width);
  assert(0 <= y && y < m_height);
  int i = (y * m_width + x) * 3;
  for (int c = 0; c < m_channels; c++) {
    m_data[i + c] = pixel[c];
  }
}

glm::dvec3 Image::sample(const glm::dvec2& uv) const { return sample(uv.x, uv.y); }

glm::dvec3 Image::sample(double u, double v) const
{
  assert(valid());
  // assert(0 <= u && u <= 1);
  // assert(0 <= v && v <= 1);

  if (!((0 <= u && u <= 1) && (0 <= v && v <= 1))) {
    return glm::dvec3(1, 0, 1);
  }

#if 1
  return glm::dvec3(u, v, 1);
#else
  // TODO: proper bilinear interpolation
  auto p = pixel(u * (m_width - 1), v * (m_height - 1));
  return rgb(p.r, p.g, p.b);
#endif
}

void Image::free_data()
{
  if (m_data) {
    STBI_FREE(m_data);
    m_data = nullptr;
  }
}

bool Image::valid() const { return (m_data != nullptr) && (0 < m_width) && (0 < m_height) && (0 < m_channels); }