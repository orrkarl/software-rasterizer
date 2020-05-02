#pragma once

#include "predef.h"

#include "converters.h"

void init(const uvec2& viewport); 

void periodic(const uvec2& viewport, float* depthBuffer, Color* colorBuffer); 

