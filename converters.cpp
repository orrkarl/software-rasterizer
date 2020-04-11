#include "converters.h"

#include <iomanip>

std::ostream& operator<<(std::ostream& os, const glm::mat4& mat) {
	auto matT = glm::transpose(mat);
	os << "mat4{" << std::endl;
	os << '\t' << matT[0] << std::endl;
	os << '\t' << matT[1] << std::endl;
	os << '\t' << matT[2] << std::endl;
	os << '\t' << matT[3] << std::endl;
	os << "}";
	return os;
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
