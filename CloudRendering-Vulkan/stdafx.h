#pragma once

#include <stdint.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <ctime>

#define NOMINMAX // disable windows min and max functions
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Validation.h"
#include "Initializers.h"