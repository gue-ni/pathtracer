#pragma once 

struct Medium {
    glm::dvec3 absorbance_coeff;
    glm::dvec3 scatter_coeff;
    glm::dvec3 scatter() const;
    bool interaction() const;
};