#include "stdafx.h"
#include "VulkanInstance.h"

#include "Initializers.h"

VulkanInstance::VulkanInstance(const VulkanConfiguration& config)
{
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
	extensions.push_back("VK_EXT_debug_report");
	
	VkApplicationInfo appInfo = initializers::CreateApplicationInfo(config);
	VkInstanceCreateInfo instanceInfo = initializers::CreateInstanceCreateInfo(appInfo, layers, extensions);

	ValidCheck(vkCreateInstance(&instanceInfo, NULL, &instance));
}

VulkanInstance::~VulkanInstance()
{
	if (instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(instance, NULL);
	}
}

VkInstance& VulkanInstance::GetInstance()
{
	return instance;
}
