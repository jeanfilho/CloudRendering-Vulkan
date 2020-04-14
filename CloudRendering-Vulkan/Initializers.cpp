#include "stdafx.h"
#include "Initializers.h"

#include "VulkanConfiguration.h"
#include "QueueFamilyIndices.h"
#include "SwapchainSupportDetails.h"

#include "GLFW/glfw3native.h"

VkInstanceCreateInfo initializers::InstanceCreateInfo(
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

VkSwapchainCreateInfoKHR initializers::SwapchainCreateInfo(VkSurfaceKHR surface, QueueFamilyIndices& indices, SwapchainSupportDetails swapchainSupport, uint32_t imageCount, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent)
{
	VkSwapchainCreateInfoKHR info{};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.minImageCount = imageCount;
	info.imageFormat = surfaceFormat.format;
	info.imageColorSpace = surfaceFormat.colorSpace;
	info.imageExtent = extent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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

VkImageViewCreateInfo initializers::ImageViewCreateInfo(VkImage image, VkImageViewType viewType, VkFormat format)
{
	VkImageViewCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.image = image;
	info.viewType = viewType;
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

VkShaderModuleCreateInfo initializers::ShaderModuleCreateInfo(std::vector<char>& code)
{
	VkShaderModuleCreateInfo info{};

	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	return info;
}

VkPipelineShaderStageCreateInfo initializers::PipelineShaderStageCreateInfo(VkShaderModule module, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.stage = stage;
	info.module = module;
	info.pName = "main";

	return info;
}

VkFramebufferCreateInfo initializers::FramebufferCreateInfo(VkRenderPass renderPass, std::vector<VkImageView>& attachments, VkExtent2D& swapchainExtent)
{
	VkFramebufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = renderPass;
	info.attachmentCount = static_cast<uint32_t>(attachments.size());
	info.pAttachments = attachments.data();
	info.width = swapchainExtent.width;
	info.height = swapchainExtent.height;
	info.layers = 1;

	return info;
}

VkDescriptorSetLayoutCreateInfo initializers::DescriptorSetLayoutCreateInfo(std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	VkDescriptorSetLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.pBindings = bindings.data();
	info.bindingCount = static_cast<uint32_t>(bindings.size());

	return info;
}

VkDescriptorSetLayoutBinding initializers::DescriptorSetLayoutBinding(uint32_t binding, VkShaderStageFlags flags, VkDescriptorType type, uint32_t count /*= 1*/)
{
	VkDescriptorSetLayoutBinding info{};

	info.binding = binding;
	info.descriptorType = type;
	info.descriptorCount = 1;
	info.stageFlags = flags;

	return info;
}

VkDescriptorSetAllocateInfo initializers::DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const VkDescriptorSetLayout* setLayouts, uint32_t descriptorSetCount)
{
	VkDescriptorSetAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = descriptorPool;
	info.pSetLayouts = setLayouts;
	info.descriptorSetCount = descriptorSetCount;

	return info;
}

VkDescriptorPoolCreateInfo initializers::DescriptorPoolCreateInfo(std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	info.pPoolSizes = poolSizes.data();
	info.maxSets = maxSets;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	return info;
}

VkDescriptorPoolSize initializers::DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
{
	VkDescriptorPoolSize size{};

	size.descriptorCount = descriptorCount;
	size.type = type;

	return size;
}

VkCommandBufferBeginInfo initializers::CommandBufferBeginInfo()
{
	VkCommandBufferBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	return info;
}

VkSubmitInfo initializers::SubmitInfo()
{
	VkSubmitInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	return info;
}

VkWriteDescriptorSet initializers::WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, uint32_t descriptorCount /*= 1*/)
{
	VkWriteDescriptorSet info{};
	info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	info.dstSet = dstSet;
	info.descriptorType = type;
	info.dstBinding = binding;
	info.pBufferInfo = bufferInfo;
	info.descriptorCount = descriptorCount;

	return info;
}

VkWriteDescriptorSet initializers::WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkBufferView* texelBufferView, uint32_t descriptorCount)
{
	VkWriteDescriptorSet info = WriteDescriptorSet(dstSet, type, binding, bufferInfo, descriptorCount);
	info.pTexelBufferView = texelBufferView;
	return info;
}

VkWriteDescriptorSet initializers::WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount /*= 1*/)
{
	VkWriteDescriptorSet info{};
	info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	info.dstSet = dstSet;
	info.descriptorType = type;
	info.dstBinding = binding;
	info.pImageInfo = imageInfo;
	info.descriptorCount = descriptorCount;

	return info;
}

VkCopyDescriptorSet initializers::CopyDescriptorSet()
{
	VkCopyDescriptorSet info{};
	info.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;

	return info;
}

VkDescriptorBufferInfo initializers::DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
	VkDescriptorBufferInfo info{};
	info.buffer = buffer;
	info.offset = offset;
	info.range = range;

	return info;
}

VkDescriptorImageInfo initializers::DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
{
	VkDescriptorImageInfo info{};
	info.sampler = sampler;
	info.imageView = imageView;
	info.imageLayout = imageLayout;
	return info;
}

VkBufferViewCreateInfo initializers::BufferViewCreateInfo(VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range, VkBufferViewCreateFlags flags)
{
	VkBufferViewCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	info.buffer = buffer;
	info.format = format;
	info.offset = offset;
	info.range = range;
	info.flags = flags;

	return info;
}

VkMemoryBarrier initializers::MemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	VkMemoryBarrier info;
	info.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	info.srcAccessMask = srcAccessMask;
	info.dstAccessMask = dstAccessMask;
	return info;
}

VkImageCreateInfo initializers::ImageCreateInfo(VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageCreateFlags flags /*= 0*/)
{
	VkImageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.flags = flags;
	info.imageType = imageType;
	info.format = format;
	info.extent = extent;
	info.mipLevels = mipLevels;
	info.arrayLayers = arrayLayers;
	info.samples = samples;
	info.usage = usage;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	return info;
}

VkSamplerCreateInfo initializers::SamplerCreateInfo()
{
	VkSamplerCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.maxAnisotropy = 1.0f;
	info.magFilter = VK_FILTER_LINEAR;
	info.minFilter = VK_FILTER_LINEAR;
	return info;
}

VkPushConstantRange initializers::PushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size)
{
	VkPushConstantRange range{};
	range.stageFlags = stageFlags;
	range.offset = offset;
	range.size = size;
	return range;
}
