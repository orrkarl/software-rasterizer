#include "converters.h"

#include <iomanip>

std::ostream& operator<<(std::ostream& os, const glm::vec2& vec) {
	return os << "vec2{" << std::setw(9) << vec.x << ", " << std::setw(9) << vec.y << "}";
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
	return os << "vec3{" << std::setw(9) << vec.x << ", " << std::setw(9) << vec.y << ", " << std::setw(9) << vec.z << "}";
}

std::ostream& operator<<(std::ostream& os, const glm::vec4& vec) {
	return os << "vec4{" << std::setw(9) << vec.x << ", " << std::setw(9) << vec.y << ", " << std::setw(9) << vec.z << ", " << std::setw(9) << vec.w << "}";
}

glm::vec4 rasterFromNDC(const glm::vec4& ndc, const glm::uvec2& viewport) {
	return { (ndc.x + ndc.w) * viewport.x / 2, (ndc.w - ndc.y) * viewport.y / 2, ndc.z, ndc.w };
}

Color mkColor(const glm::vec4& color) {
	unsigned char r = static_cast<unsigned char>(255 * color.x);
	unsigned char g = static_cast<unsigned char>(255 * color.y);
	unsigned char b = static_cast<unsigned char>(255 * color.z);
	unsigned char a = static_cast<unsigned char>(255 * color.w);
	return {r, g, b, a};
}

Color mkColor(const glm::vec3& color) {
	return mkColor({color, 1.0f});
}
