#pragma once

#include "predef.h"
#include "user_data.h"
#include "converters.h"

void rasterTriangleIndexed(const uvec2& viewport, const std::vector<vec3>& vertecies, const std::vector<vec3>& colors, const std::vector<std::array<uint32_t, 3>>& indices, const mat4& mvp, float* depthBuffer, Color* colorBuffer); 

