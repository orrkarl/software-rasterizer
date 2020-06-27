#include "predef.h"

#pragma once

using imat3 = mat<3, 3, int32_t, glm::highp>;
using ivec3 = glm::ivec3;
using i32 = glm::i32;
using i64 = glm::i64;
using lmat3 = mat<3, 3, int64_t, glm::highp>;
using lvec3 = vec<3, int64_t>;
using u16vec2 = glm::u16vec2;

template <typename T>
struct DetermineDimension;

template <typename T>
constexpr auto DetermineDimensionValue = DetermineDimension<T>::Value;

#include "TypeUtil.inl"
