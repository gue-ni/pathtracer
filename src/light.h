#pragma once 

#include "geometry.h"

struct Light : public Primitive {
    glm::dvec3 random_point_on_light() const;
    double area() const;
};
