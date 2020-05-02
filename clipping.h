#pragma once

#include "predef.h"

struct Triangle {
	vec4 v0, v1, v2;
};

size_t clip(const Triangle& tri, std::array<vec2, 9>& coeffs);

