#include <gtest/gtest.h>

#include "material.h"
#include "util.h"

TEST(material, test_01)
{
  constexpr glm::dvec3 n(0, 1, 0);
  glm::dvec3 v = glm::normalize(glm::dvec3(0.5, 0.5, 0.5));

  glm::mat3 m = local_to_world(n);
}
