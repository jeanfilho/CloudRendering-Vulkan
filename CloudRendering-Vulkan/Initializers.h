#pragma once

//Fwd. decl.
class VulkanDevice;
struct VulkanConfiguration;
struct QueueFamilyIndices;

namespace initializers
{
	VkInstanceCreateInfo CreateInstanceCreateInfo(
		const VkApplicationInfo& appInfo,
		const std::vector<const char*>& layers,
		const std::vector<const char*>& extensions);

	VkApplicationInfo ApplicationInfo(const VulkanConfiguration& config);

	VkDeviceQueueCreateInfo DeviceQueueCreateInfo(uint32_t queueFamilyIndex, float& priority);

	VkDeviceCreateInfo DeviceCreateInfo(std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, VkPhysicalDeviceFeatures& deviceFeatures, std::vector<const char*>& extensions);

	VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count);

	VkBufferCreateInfo BufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage);

	VkMemoryAllocateInfo MemoryAllocateInfo(VkDeviceSize size, uint32_t memoryTypeIndex);

	VkWin32SurfaceCreateInfoKHR Win32SurfaceCreateInfo(GLFWwindow* window);

	VkSwapchainCreateInfoKHR SwapchainCreateInfo(VulkanDevice* device, uint32_t imageCount, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent);

	VkImageViewCreateInfo ImageViewCreateInfo(VkImage image, VkFormat format);
}