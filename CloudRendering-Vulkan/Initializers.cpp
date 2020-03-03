#include "stdafx.h"
#include "Initializers.h"

#include "VulkanConfiguration.h"
#include "QueueFamilyIndices.h"
#include "VulkanDevice.h"

#include "GLFW/glfw3native.h"

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

VkApplicationInfo initializers::ApplicationInfo(const VulkanConfiguration& config)
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

VkDeviceQueueCreateInfo initializers::DeviceQueueCreateInfo(uint32_t queueFamilyIndex, float& priority)
{
	VkDeviceQueueCreateInfo queueCreateInfo{};

	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &priority;

	return queueCreateInfo;
}

VkDeviceCreateInfo initializers::DeviceCreateInfo(std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, VkPhysicalDeviceFeatures& deviceFeatures, std::vector<const char*>& deviceExtensions)
{
	VkDeviceCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.pQueueCreateInfos = queueCreateInfos.data();
	info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	info.pEnabledFeatures = &deviceFeatures;

	info.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	info.ppEnabledExtensionNames = deviceExtensions.data();

	return info;
}

VkCommandPoolCreateInfo initializers::CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/)
{
	VkCommandPoolCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;

	return info;
}

VkCommandBufferAllocateInfo initializers::CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count)
{
	VkCommandBufferAllocateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = pool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = count;

	return info;
}

VkBufferCreateInfo initializers::BufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage)
{
	VkBufferCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.size = size;
	info.usage = usage;

	return info;
}

VkMemoryAllocateInfo initializers::MemoryAllocateInfo(VkDeviceSize size, uint32_t memoryTypeIndex)
{
	VkMemoryAllocateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.allocationSize = size;
	info.memoryTypeIndex = memoryTypeIndex;

	return info;
}

VkWin32SurfaceCreateInfoKHR initializers::Win32SurfaceCreateInfo(GLFWwindow* window)
{
	VkWin32SurfaceCreateInfoKHR info{};

	info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hwnd = glfwGetWin32Window(window);
	info.hinstance = GetModuleHandle(nullptr);

	return info;
}

VkSwapchainCreateInfoKHR initializers::SwapchainCreateInfo(VulkanDevice* device, uint32_t imageCount, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent)
{

	QueueFamilyIndices& indices = device->GetPhysicalDevice()->GetQueueFamilyIndices();
	VkSurfaceKHR surface = device->GetSurface()->GetSurface();
	SwapchainSupportDetails swapchainSupport = device->GetPhysicalDevice()->QuerySwapchainSupport(device->GetSurface());

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.minImageCount = imageCount;
	info.imageFormat = surfaceFormat.format;
	info.imageColorSpace = surfaceFormat.colorSpace;
	info.imageExtent = extent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		info.queueFamilyIndexCount = 2;
		info.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.queueFamilyIndexCount = 0; // Optional
		info.pQueueFamilyIndices = nullptr; // Optional
	}

	info.preTransform = swapchainSupport.capabilities.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = presentMode;
	info.clipped = VK_TRUE;

	info.oldSwapchain = VK_NULL_HANDLE;

	return info;
}

VkImageViewCreateInfo initializers::ImageViewCreateInfo(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.image = image;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = format;

	info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;

	return info;
}
