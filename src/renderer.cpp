#include "renderer.h"
#include "geometry.h"
#include "material.h"
#include "util.h"
#include <glm/gtc/type_ptr.hpp>

#define DEBUG_NORMAL     0
#define RUSSIAN_ROULETTE 1

std::atomic<uint64_t> bounce_counter = 0;

static uint8_t map_pixel(double color) { return static_cast<uint8_t>(glm::clamp(color, 0.0, 1.0) * 255.0); }

static glm::u8vec3 map_pixel(const glm::dvec3 color)
{
  return {map_pixel(color.r), map_pixel(color.g), map_pixel(color.b)};
}

static glm::dvec3 normal_as_color(const glm::dvec3& N) { return 0.5 * glm::dvec3(N.x + 1, N.y + 1, N.z + 1); }

Renderer::Renderer(Camera* camera, Scene* scene) : m_camera(camera), m_scene(scene)
{
  m_buffer = new glm::dvec3[m_camera->width() * m_camera->height()];

  for (int y = 0; y < m_camera->height(); y++) {
    for (int x = 0; x < m_camera->width(); x++) {
      m_buffer[y * m_camera->width() + x] = glm::dvec3(0.0);
    }
  }
}

Renderer::~Renderer()
{
  if (m_buffer != nullptr) delete[] m_buffer;
}

void Renderer::render_old(int samples, int max_bounce, bool print_progress)
{
  double sample_weight = 1.0 / double(samples);

#pragma omp parallel for schedule(dynamic, 1)
  for (int y = 0; y < m_camera->height(); y++) {
    if (print_progress) {
      printf("Progress: %.2f%%\n", (double(y) / double(m_camera->height())) * 100.0);
    }

    for (int x = 0; x < m_camera->width(); x++) {
      glm::dvec3 color(0.0);

      for (int s = 0; s < samples; s++) {
        Ray ray = m_camera->get_ray(x, y);
        color += trace_ray(ray, 0, max_bounce);
      }

      m_buffer[y * m_camera->width() + x] = color * sample_weight;
    }
  }
}

void Renderer::render(int samples, int max_bounce, bool print_progress)
{
  double sample_weight = 1.0 / double(samples);

#pragma omp parallel for schedule(dynamic, 1)
  for (int y = 0; y < m_camera->height(); y++) {
    if (print_progress) {
      printf("Progress: %.2f%%\n", (double(y) / double(m_camera->height())) * 100.0);
    }

    for (int x = 0; x < m_camera->width(); x++) {
      int i = y * m_camera->width() + x;
      glm::dvec3 result = m_buffer[i];

      for (int s = 0; s < samples; s++) {
        Ray ray = m_camera->get_ray(x, y);
        auto color = trace_ray(ray, 0, max_bounce);
        result = glm::mix(result, color, 1.0 / double(total_samples + s + 1));
      }

      m_buffer[i] = result;
    }
  }

  total_samples += samples;
}

// https://en.wikipedia.org/wiki/Grayscale#Luma_coding_in_video_systems
static double luma(const glm::dvec3& color) { return glm::dot(color, glm::dvec3(0.2126, 0.7152, 0.0722)); }

glm::dvec3 Renderer::trace_ray(const Ray& ray, int depth, int max_depth)
{
  if (max_depth <= depth) {
    return glm::vec3(0);
  }

  bounce_counter++;

  auto possible_hit = m_scene->find_intersection(ray);

  if (!possible_hit.has_value()) {
    return m_scene->background(ray);
  }

  Intersection surface = possible_hit.value();
  Material* material = surface.material;

#if DEBUG_NORMAL
  return normal_as_color(surface.normal);
#endif

#if RUSSIAN_ROULETTE
  // russian roulette
  double rr_weight = 1;
  int min_depth = 3;

  if (min_depth < depth) {
    double rr_prob = luma(surface.albedo());
    if (random_double() >= rr_prob) {
      return surface.material->emission;
    } else {
      rr_weight = 1.0 / rr_prob;
    }
  }
#else
  double rr_weight = 1;
#endif

  glm::mat3 local2world = local_to_world(surface.normal);
  glm::mat3 world2local = glm::inverse(local2world);

  BxDF brdf(&surface);

  glm::dvec3 wo = world2local * (-ray.direction);
  glm::dvec3 wi = brdf.sample(wo);

  Ray outgoing(surface.point, local2world * wi);

  glm::dvec3 radiance(0.0);

  if (depth == 0) {
    radiance += material->emission;
  }

  glm::dvec3 indirect_light = trace_ray(outgoing, depth + 1, max_depth);

  glm::dvec3 direct_light(0.0);

  if (0 < m_scene->num_lights()) {
    direct_light = sample_lights(surface.point, brdf);
  }

  return radiance + direct_light + indirect_light * rr_weight * brdf.eval(wo, wi);
}

// https://computergraphics.stackexchange.com/questions/5152/progressive-path-tracing-with-explicit-light-sampling
// https://computergraphics.stackexchange.com/questions/4288/path-weight-for-direct-light-sampling
glm::dvec3 Renderer::sample_lights(const glm::dvec3& point, const BxDF& bsdf)
{
#if 1
  glm::dvec3 direct_light(0.0);

  Primitive light = m_scene->random_light();

  glm::dvec3 point_to_light = light.sample_point() - point;

  double distance = glm::length(point_to_light);

  point_to_light /= distance;

  auto record = m_scene->find_intersection(Ray(point, point_to_light));

  if (record.has_value()) {
    Intersection surface = record.value();
    if (surface.id == light.id) {
      glm::dvec3 wi, wo;  // TODO

      glm::dvec3 emission = light.material->emission;

      double pdf = 1.0 / double(m_scene->num_lights());
      double LoN = glm::max(glm::dot(light.triangle.normal(), -point_to_light), 0.0);
      double weight = (light.area() / sq(distance)) * LoN;

      return (emission * weight * bsdf.eval(wo, wi)) / pdf;
    }
  }

  return glm::dvec3(0);

#else
  return glm::dvec3(0);
#endif
}

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
glm::dvec3 aces_tone_map(glm::dvec3 x)
{
  const double a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
  return (x * (a * x + b)) / (x * (c * x + d) + e);
}

void Renderer::save_image(const std::filesystem::path& path)
{
  Image output(m_camera->width(), m_camera->height(), 3);

  for (int y = 0; y < m_camera->height(); y++) {
    for (int x = 0; x < m_camera->width(); x++) {
      glm::dvec3 color = m_buffer[y * m_camera->width() + x];
      color = aces_tone_map(color);
      color = gamma_correction(color);
      glm::u8vec3 pixel = map_pixel(color);
      output.set_pixel(x, y, glm::value_ptr(pixel));
    }
  }

  output.write(path);

  auto now = std::chrono::system_clock::now();
  std::time_t current_time = std::chrono::system_clock::to_time_t(now);

  std::cout << "[" << std::put_time(std::localtime(&current_time), "%Y-%m-%d %H:%M:%S") << "] Save image to " << path
            << std::endl;
}
