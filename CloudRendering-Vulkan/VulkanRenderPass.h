#pragma once

class VulkanDevice;
class VulkanSwapchain;

class VulkanRenderPass
{
public:
	VulkanRenderPass(VulkanDevice* device, VulkanSwapchain* swapchain);
	~VulkanRenderPass();

	VkRenderPass GetRenderPass();

private:
	VulkanDevice* m_device = nullptr;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
};