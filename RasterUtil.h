#pragma once

#include "predef.h"

#include "TypeUtil.h"

template <typename T, glm::qualifier Q>
mat<3, 3, T, Q> adjoint(mat<3, 3, T, Q> const& m);

#include "RasterUtil.inl"

