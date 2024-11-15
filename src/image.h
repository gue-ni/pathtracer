#pragma once

#include <string>
#include <filesystem>
#include <glm/glm.hpp>

class Image
{
 public:
  Image();
  Image(int width, int height, int channels);
  ~Image();
  bool load(const std::filesystem::path& path);
  bool write(const std::filesystem::path& path) const;
  glm::dvec3 sample(double u, double v, bool interpolate = true) const;
  glm::dvec3 sample(const glm::dvec2& uv) const;
  int width() const { return m_width; }
  int height() const { return m_height; }
  int channels() const { return m_channels; }
  void set_pixel(int x, int y, unsigned char* pixel);
  bool valid() const;

 private:
  unsigned char* m_data;
  int m_width, m_height, m_channels;
  glm::u8vec3 pixel(int x, int y) const;
  void free_data();
};
