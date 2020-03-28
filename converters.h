#pragma once

#include <glm/glm.hpp>
#include <iostream>

struct Color {
	unsigned char r, g, b, a;
};

std::ostream& operator<<(std::ostream& os, const glm::vec2& vec); 
std::ostream& operator<<(std::ostream& os, const glm::vec3& vec); 
std::ostream& operator<<(std::ostream& os, const glm::vec4& vec); 

glm::vec4 rasterFromNDC(const glm::vec4& ndc, const glm::uvec2& viewport); 

Color mkColor(const glm::vec4& color); 

Color mkColor(const glm::vec3& color); 

