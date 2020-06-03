#pragma once

//Fwd. decl.
struct VulkanConfiguration;
struct QueueFamilyIndices;
struct SwapchainSupportDetails;

namespace initializers
{
	VkInstanceCreateInfo InstanceCreateInfo(
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

	VkSwapchainCreateInfoKHR SwapchainCreateInfo(VkSurfaceKHR surface, QueueFamilyIndices& indices, SwapchainSupportDetails swapchainSupport, uint32_t imageCount, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent);

	VkImageViewCreateInfo ImageViewCreateInfo(VkImage image, VkImageViewType viewType, VkFormat format);

	VkImageCreateInfo ImageCreateInfo(VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageCreateFlags flags = 0);

	VkSamplerCreateInfo SamplerCreateInfo();

	VkPushConstantRange PushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size);

	VkShaderModuleCreateInfo ShaderModuleCreateInfo(std::vector<char>& code);

	VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderModule module, VkShaderStageFlagBits stage);

	VkFramebufferCreateInfo FramebufferCreateInfo(VkRenderPass renderPass, std::vector<VkImageView>& attachments, VkExtent2D& swapchainExtent);

	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(std::vector<VkDescriptorSetLayoutBinding>& bindings);

	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkShaderStageFlags flags, VkDescriptorType type, uint32_t count = 1);

	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const VkDescriptorSetLayout* setLayouts, uint32_t descriptorSetCount);

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);

	VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t count);

	VkCommandBufferBeginInfo CommandBufferBeginInfo();

	VkSubmitInfo SubmitInfo();

	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, uint32_t descriptorCount = 1);

	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkBufferView* texelBufferView, uint32_t descriptorCount = 1);

	VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount = 1);

	VkCopyDescriptorSet CopyDescriptorSet();

	VkDescriptorBufferInfo DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

	VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);

	VkBufferViewCreateInfo BufferViewCreateInfo(VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range, VkBufferViewCreateFlags flags);

	VkMemoryBarrier MemBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

	VkMappedMemoryRange MappedMemoryRange(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size);
}