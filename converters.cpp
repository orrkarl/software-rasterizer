#include "converters.h"

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
	return os << "glm::vec3{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
}

glm::vec3 viewportFromNDC(const glm::vec3& ndc, const glm::uvec2& viewport) {
	return { (ndc.x + 1.0f) * viewport.x / 2, (ndc.y + 1.0f) * viewport.y / 2, ndc.z };
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
