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
	VkDeviceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.pQueueCreateInfos = queueCreateInfos.data();
	info.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	info.pEnabledFeatures = &deviceFeatures;

	return info;
}

VkCommandPoolCreateInfo initializers::CreateCommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/)
{
	VkCommandPoolCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;

	return info;
}

VkCommandBufferAllocateInfo initializers::CreateCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count)
{
	VkCommandBufferAllocateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = pool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = count;

	return info;
}
