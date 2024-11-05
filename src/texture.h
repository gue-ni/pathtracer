#pragma once

class Texture2D {
public:
  bool load(const std::string& path);
  glm::dvec3 sample(double u, double v) const;
private:
   int m_width, m_height;
   unsigned char* m_data;
};
