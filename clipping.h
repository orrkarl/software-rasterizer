#pragma once

#include "predef.h"

struct Triangle {
	vec4 v0, v1, v2;
};

std::vector<Triangle> clip(const Triangle& tri);

