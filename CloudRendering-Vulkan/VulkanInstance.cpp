#include "stdafx.h"
#include "VulkanInstance.h"

#include "Initializers.h"

VulkanInstance::VulkanInstance(const VulkanConfiguration& config)
{
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
