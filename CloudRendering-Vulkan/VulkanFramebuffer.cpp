#include "stdafx.h"
#include "VulkanFrameBuffer.h"

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanImageView.h"
#include "VulkanSwapchain.h"

VulkanFramebuffer::VulkanFramebuffer(VulkanDevice* device, VulkanRenderPass* renderPass, VulkanImageView* swapchainImageView, VulkanSwapchain* swapchain)
{
	m_device = device;

	VkImageView attachments[] = { swapchainImageView->GetImageView() };
	VkFramebufferCreateInfo framebufferInfo = initializers::FramebufferCreateInfo(renderPass->GetRenderPass(), attachments, swapchain->GetExtent());

	ValidCheck(vkCreateFramebuffer(m_device->GetDevice(), &framebufferInfo, nullptr, &m_framebuffer));
}

VulkanFramebuffer::~VulkanFramebuffer()
{
	if (m_framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(m_device->GetDevice(), m_framebuffer, nullptr);
	}
}

VkFramebuffer VulkanFramebuffer::GetFramebuffer()
{
	return m_framebuffer;
}
