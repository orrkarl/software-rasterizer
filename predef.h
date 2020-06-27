#pragma once

#define GLM_FORCE_UNRESTRICTED_GENTYPE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/detail/qualifier.hpp>

#include <algorithm>
#include <array> 
#include <chrono>
#include <cmath>
#include <functional>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <iomanip>
#include <limits>
#include <thread>
#include <type_traits>
#include <vector>

using glm::mat;
using glm::mat4;
using glm::vec;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::uvec2;

float sum(const vec3& vec);

