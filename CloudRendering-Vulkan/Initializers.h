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

	VkCommandPoolCreateInfo CreateCommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo CreateCommandBufferAllocateInfo(VkCommandPool pool, uint32_t count);

	VkBufferCreateInfo CreateBufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage);

	VkMemoryAllocateInfo CreateMemoryAllocateInfo(VkDeviceSize size, uint32_t memoryTypeIndex);
}