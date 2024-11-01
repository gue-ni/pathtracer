#include "geometry.h"
#include <cstdio>
#include <optional>
#include <glm/glm.hpp>
#include "glm/exponential.hpp"

bool ray_vs_sphere(const Ray& r, const Sphere& s, float& t)
{
  glm::dvec3 m = r.origin - s.center;
  double b = glm::dot(m, r.direction);
  double c = glm::dot(m, m) - s.radius * s.radius;

  if (c > 0.0 && b > 0.0) return false;
  double discr = b * b - c;

  if (discr < 0.0) return false;

  t = -b - glm::sqrt(discr);
  if (t < 0.0) {
    // actually this should not be happening
    t = 0.0;
  }

  return true;
}

bool hit(const Ray& r const Sphere& s) {
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
}

bool ray_vs_triangle(const Ray&, const Triangle&, float& t) { return false; }

std::optional<Intersection> Primitive::intersect(const Ray& ray) const
{
  float t;
  switch (type) {
    case SPHERE: {
      if (ray_vs_sphere(ray, sphere, t)) {
        Intersection surface;
        surface.hit = true;
        surface.t = t;
        surface.point = ray.point_at(t);
        surface.normal = glm::normalize(surface.point - sphere.center);
        surface.material = material;
        return surface;
      } else {
        return std::nullopt;
      }
    }
    default:
      return std::nullopt;
  }
}
