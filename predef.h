#pragma once

#define GLM_FORCE_UNRESTRICTED_GENTYPE
#define GLM_ENABLE_EXPERIMENTAL

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

using namespace glm;

float sum(const vec3& vec);

