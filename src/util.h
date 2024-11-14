#pragma once

#include <glm/glm.hpp>
#include <random>
#include <array>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

constexpr double pi = 3.14159265359;

// x^2
constexpr inline double sq(double x) { return x * x; }

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

template <typename T>
inline T map_range(const T& value, const Interval<T>& in, const Interval<T>& out)
{
  return out.min + (value - in.min) * (out.max - out.min) / (in.max - in.min);
}

template <typename T>
inline T map_range(const T& value, const T& in_min, const T& in_max, const T& out_min, const T& out_max)
{
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

template <typename Byte>
inline glm::dvec3 rgb(Byte r, Byte g, Byte b)
{
  return glm::dvec3(double(r), double(g), double(b)) / 255.0;
}

template <typename RGB>
inline glm::dvec3 rgb(const RGB& color)
{
  return rgb(color.x, color.y, color.z);
}

// https://gamedev.stackexchange.com/questions/120352/extract-a-rotation-matrix-given-a-camera-direction-vector-and-a-up-vector-for
inline glm::dmat3 local_to_world(const glm::dvec3& up)
{
  constexpr glm::dvec3 world_up(0, 1, 0);

  constexpr double epsilon = 0.0001;
  if (glm::all(glm::epsilonEqual(up, world_up, epsilon))) {
    return glm::mat3(1.0);  // identity
  }

  glm::dvec3 right = glm::cross(up, world_up);
  glm::dvec3 forward = glm::cross(-right, up);
  return glm::dmat3(right, up, forward);
}

inline double random_double()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dist(0.0f, 1.0f);
  return dist(gen);
}

// theta is polar angle
// phi is azimuth angle (angle around polar axis)
// https://ameye.dev/notes/sampling-the-hemisphere/
inline glm::dvec3 spherical_to_cartesian(double theta, double phi)
{
  double x = std::cos(phi) * std::sin(theta);
  double y = std::cos(theta);
  double z = std::sin(phi) * std::sin(theta);
  return {x, y, z};
}

inline glm::dvec3 random_unit_vector()
{
  double phi = 2.0 * pi * random_double();
  double theta = std::acos(1.0 - 2.0 * random_double());
  return spherical_to_cartesian(theta, phi);
}

inline glm::dvec3 random_in_unit_disk()
{
  while (true) {
    double x0 = map_range(random_double(), 0.0, 1.0, -1.0, 1.0);
    double x1 = map_range(random_double(), 0.0, 1.0, -1.0, 1.0);
    auto p = glm::dvec3(x0, x1, 0);
    if (glm::dot(p, p) < 1) return p;
  }
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
#if 0
  return glm::normalize(normal + random_unit_vector());
#else
  double r0 = random_double(), r1 = random_double();
  double phi = 2.0 * pi * r0;
  double theta = std::acos(std::sqrt(r1));
  return local_to_world(normal) * spherical_to_cartesian(theta, phi);
#endif
}

// convert from linear space to gamma space
inline glm::dvec3 gamma_correction(const glm::dvec3 color, double gamma = 2.2)
{
  return glm::pow(color, glm::dvec3(1.0 / gamma));
}

// convert from gamma space to linear space
inline glm::dvec3 reverse_gamma_correction(const glm::dvec3 color, double gamma = 2.2)
{
  return glm::pow(color, glm::dvec3(gamma));
}
