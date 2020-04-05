#pragma once

class VulkanDevice;
class VulkanSwapchain;

class VulkanRenderPass
{
protected:
	VulkanRenderPass();

public:
	VulkanRenderPass(VulkanDevice* device, VulkanSwapchain* swapchain);
	~VulkanRenderPass();

	VkRenderPass GetRenderPass();

protected:
	virtual void AllocateResources(VulkanSwapchain* swapchain);

protected:
	VulkanDevice* m_device = nullptr;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
};