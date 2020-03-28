#include <glm/glm.hpp>
#include <iostream>
#include <raylib.h>
#include <vector>

using namespace glm;

std::ostream& operator<<(std::ostream& os, const vec3& vec) {
	return os << "vec3{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
}

vec3 viewportFromNDC(const vec3& ndc, const uvec2& viewport) {
	return { (ndc.x + 1.0f) * viewport.x / 2, (ndc.y + 1.0f) * viewport.y / 2, ndc.z };
}

Color mkColor(const vec4& color) {
	unsigned char r = static_cast<unsigned char>(255 * color.x);
	unsigned char g = static_cast<unsigned char>(255 * color.y);
	unsigned char b = static_cast<unsigned char>(255 * color.z);
	unsigned char a = static_cast<unsigned char>(255 * color.w);
	return {r, g, b, a};
}

Color mkColor(const vec3& color) {
	return mkColor({color, 1.0f});
}

int main() {
	const uvec2 VIEWPORT{640, 480};

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(VIEWPORT.x, VIEWPORT.y, "software rasterization");
	SetTargetFPS(60);                           

	vec3 v0(-0.5, 0.5, 1.0);
	vec3 v1(0.5, 0.5, 1.0);
	vec3 v2(0.0, -0.5, 1.0);

	v0 = viewportFromNDC(v0, VIEWPORT); 
	v1 = viewportFromNDC(v1, VIEWPORT); 
	v2 = viewportFromNDC(v2, VIEWPORT); 

	mat3 vertexMatrix{
		{ v0.x, v1.x, v2.x },
		{ v0.y, v1.y, v2.y },
		{ v0.z, v1.z, v2.z }
	};

	// Compute the inverse of vertex matrix to use it for setting up edge functions
	auto edgeMatrix = inverse(vertexMatrix);

	// Calculate edge functions based on the vertex matrix
	auto e0 = edgeMatrix[0];
	auto e1 = edgeMatrix[1];
	auto e2 = edgeMatrix[2];

	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(BLACK);

		for (auto y = 0; y < VIEWPORT.y; ++y) {
			for (auto x = 0; x < VIEWPORT.x; ++x) {
				vec3 sample{x + 0.5f, y + 0.5f, 1.0f};

				auto inside0 = dot(e0, sample) >= 0.0f;
				auto inside1 = dot(e1, sample) >= 0.0f;
				auto inside2 = dot(e2, sample) >= 0.0f;

				if (inside0 && inside1 && inside2) {
					DrawPixel(x, y, mkColor({1.0f, 0.0f, 0.0f}));
				}
			}
		}

		EndDrawing();
	}

	CloseWindow();
}
