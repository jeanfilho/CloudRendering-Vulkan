#pragma once

//Fwd. decl.
struct VulkanConfiguration;
struct QueueFamilyIndices;

namespace initializers
{
	VkInstanceCreateInfo CreateInstanceCreateInfo(
		const VkApplicationInfo& appInfo,
		const std::vector<const char*>& layers,
		const std::vector<const char*>& extensions);

	VkApplicationInfo CreateApplicationInfo(const VulkanConfiguration& config);

	VkDeviceQueueCreateInfo CreateDeviceQueueCreateInfo(uint32_t queueFamilyIndex, float& priority);

	VkDeviceCreateInfo CreateDeviceCreateInfo(std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, VkPhysicalDeviceFeatures& deviceFeatures);

}