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
#include <thread>

#define NOMINMAX // disable windows min and max functions
#define VK_USE_PLATFORM_WIN32_KHR
#pragma warning(push, 0) 
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"
#pragma warning(pop) 

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

#include "Validation.h"
#include "Initializers.h"
#include "Utilities.h"