#pragma once

#include <glm/glm.hpp>
#include <random>

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

inline glm::dvec3 rgb(int r, int g, int b) { return glm::dvec3(double(r), double(g), double(b)) / 255.0; }

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

template <typename T>
inline T map_range(const T& value, const T& in_min, const T& in_max, const T& out_min, const T& out_max)
{
  return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}