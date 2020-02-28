#include "stdafx.h"
#include "Initializers.h"

#include "VulkanConfiguration.h"
#include "QueueFamilyIndices.h"

VkInstanceCreateInfo initializers::CreateInstanceCreateInfo(
	const VkApplicationInfo& appInfo,
	const std::vector<const char*>& layers,
	const std::vector<const char*>& extensions)
{
	VkInstanceCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	info.pApplicationInfo = &appInfo;

	info.enabledExtensionCount = (uint32_t)extensions.size();
	info.ppEnabledExtensionNames = extensions.data();

	info.enabledLayerCount = (uint32_t)layers.size();
	info.ppEnabledLayerNames = layers.data();

	return info;
}

VkApplicationInfo initializers::CreateApplicationInfo(const VulkanConfiguration& config)
{
	VkApplicationInfo info{};

	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

	info.pApplicationName = config.applicationName;
	info.applicationVersion = config.applicationVersion;

	info.pEngineName = config.engineName;
	info.engineVersion = config.engineVersion;

	info.apiVersion = config.apiVersion;

	return info;
}

VkDeviceQueueCreateInfo initializers::CreateDeviceQueueCreateInfo(uint32_t queueFamilyIndex, float& priority)
{
	VkDeviceQueueCreateInfo queueCreateInfo{};

	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &priority;

	return queueCreateInfo;
}

VkDeviceCreateInfo initializers::CreateDeviceCreateInfo(std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, VkPhysicalDeviceFeatures& deviceFeatures)
{
	return VkDeviceCreateInfo();
}
