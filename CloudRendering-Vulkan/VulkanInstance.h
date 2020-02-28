#pragma once

#include "VulkanConfiguration.h"

class VulkanInstance
{
public:
	VulkanInstance(const VulkanConfiguration& config);
	~VulkanInstance();

	VkInstance& GetInstance();

private:
	VkInstance instance;
	std::vector<const char*> layers;
	std::vector<const char*> extensions;
};