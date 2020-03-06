#pragma once

// Fwd Decl.
class VulkanDevice;
class VulkanSwapchain;

class VulkanPipelineLayout
{
public:
	VulkanPipelineLayout(VulkanDevice* device, VulkanSwapchain* swapchain);
	~VulkanPipelineLayout();

	VkPipelineLayout& GetPipelineLayout();

private:
	VulkanDevice* m_device = nullptr;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
};