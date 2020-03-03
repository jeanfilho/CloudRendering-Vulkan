#pragma once

#include "VulkanConfiguration.h"

class VulkanInstance
{
public:
	VulkanInstance(const VulkanConfiguration& config);
	~VulkanInstance();

	VkInstance& GetInstance();

private:
	VkInstance m_instance;
	std::vector<const char*> m_layers;
	std::vector<const char*> m_extensions;
};