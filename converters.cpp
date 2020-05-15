#include "converters.h"

#include <iomanip>

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
