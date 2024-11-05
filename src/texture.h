#pragma once

#include <string>
#include <filesystem>
#include <glm/glm.hpp>

class Texture2D
{
 public:
  Texture2D();
  ~Texture2D();
  bool load(const std::filesystem::path& path);
  glm::dvec3 sample(double u, double v) const;
  glm::dvec3 sample(const glm::dvec2& uv) const;

 private:
  unsigned char* m_data;
  int m_width, m_height, m_channels;
  glm::u8vec3 pixel(int x, int y) const;
};
