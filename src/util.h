#pragma once

#include <glm/glm.hpp>
#include <random>
#include <array>

constexpr double pi = 3.14159265359;

template <typename T>
struct Interval {
  T min, max;
  Interval(const T& _min, const T& _max) : min(_min), max(_max) {}
  bool contains(const T& value) const { return min <= value && value <= max; }
  bool surrounds(const T& value) const { return min < value && value < max; }
  T clamp(const T& value) const
  {
    if (max < value) return max;
    if (value < min) return min;
    return value;
  }
  Interval<T> expand(double delta) const
  {
    double padding = delta / 2;
    return Interval<T>(min - padding, max + padding);
  }
};

template <typename Byte>
inline glm::dvec3 rgb(Byte r, Byte g, Byte b)
{
  return glm::dvec3(double(r), double(g), double(b)) / 255.0;
}

inline double random_double()
{
#if 0
  return (double)rand() / ((double)RAND_MAX + 1);
#else
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dist(0.0f, 1.0f);
  return dist(gen);
#endif
}

inline glm::dvec3 random_unit_vector()
{
  double theta = random_double() * 2.0f * pi;
  double phi = std::acos(1.0f - 2.0f * random_double());

  double x = std::sin(phi) * std::cos(theta);
  double y = std::sin(phi) * std::sin(theta);
  double z = std::cos(phi);

  return glm::dvec3(x, y, z);
}

inline glm::dvec3 uniform_hemisphere_sampling(const glm::dvec3& normal)
{
  glm::dvec3 unit_vector = random_unit_vector();
  if (glm::dot(unit_vector, normal) > 0.0) {
    return unit_vector;
  } else {
    return -unit_vector;
  }
}

inline glm::dvec3 cosine_weighted_sampling(const glm::dvec3& normal)
{
  return glm::normalize(normal + random_unit_vector());
}

template <typename T>
inline T map_range(const T& value, const T& in_min, const T& in_max, const T& out_min, const T& out_max)
{
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

class OrthonormalBasis
{
 public:
  OrthonormalBasis(const glm::dvec3& normal) {}
  inline glm::dvec3 transform(const glm::dvec3& vec) const;

 private:
  std::array<glm::dvec3, 3> axis;
};

inline glm::dvec3 gamma_correction(const glm::dvec3 color, double gamma = 2.2) 
{
  return glm::pow(color, glm::dvec3(1.0 / gamma));
}

inline glm::dvec3 reverse_gamma_correction(const glm::dvec3 color, double gamma = 2.2)
{
  return glm::pow(color, glm::dvec3(gamma));
}
