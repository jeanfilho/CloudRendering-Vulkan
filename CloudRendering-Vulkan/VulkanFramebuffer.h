#pragma once

class VulkanDevice;
class VulkanRenderPass;
class VulkanImageView;
class VulkanSwapchain;

class VulkanFramebuffer
{
public:
	VulkanFramebuffer(VulkanDevice* device, VulkanRenderPass* renderPass, VulkanImageView* swapchainImageView, VulkanSwapchain* swapchain);
	~VulkanFramebuffer();

	VkFramebuffer GetFramebuffer();

private:
	VulkanDevice* m_device = nullptr;
	VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
};