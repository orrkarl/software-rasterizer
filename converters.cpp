#include "converters.h"

#include <iomanip>

glm::vec4 rasterFromNDC(const glm::vec4& ndc, const glm::uvec2& viewport) {
	return glm::vec4{ (1 + ndc.x / ndc.w) * viewport.x / 2, (1 - ndc.y / ndc.w) * viewport.y / 2, ndc.z / ndc.w, ndc.w };
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
