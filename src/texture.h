#pragma once

#include <string>
#include <filesystem>
#include <glm/glm.hpp>

class Texture2D
{
 public:
  Texture2D();
  Texture2D(int width, int height, int channels);
  ~Texture2D();
  bool load(const std::filesystem::path& path);
  bool write(const std::filesystem::path& path) const;
  glm::dvec3 sample(double u, double v) const;
  glm::dvec3 sample(const glm::dvec2& uv) const;
  int width() const { return m_width; }
  int height() const { return m_height; }
  int channels() const { return m_channels; }
  void write_pixel(int x, int y, unsigned char* pixel);
  bool valid() const { return m_data && m_width && m_height && m_channels; }

 private:
  unsigned char* m_data;
  int m_width, m_height, m_channels;
  glm::u8vec3 pixel(int x, int y) const;
};
