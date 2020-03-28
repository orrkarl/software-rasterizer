#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <raylib.h>

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec); 

glm::vec3 viewportFromNDC(const glm::vec3& ndc, const glm::uvec2& viewport); 

Color mkColor(const glm::vec4& color); 

Color mkColor(const glm::vec3& color); 

