#include "predef.h"

#pragma once

using lmat3 = mat<3, 3, int64_t, glm::highp>;
using imat3 = mat<3, 3, int32_t, glm::highp>;
using lvec3 = vec<3, int64_t>;

template <typename T>
struct DetermineDimension;

template <typename T>
constexpr auto DetermineDimensionValue = DetermineDimension<T>::Value;

#include "TypeUtil.inl"
