#include "stdafx.h"
#include "VulkanRenderPass.h"

#include "VulkanSwapchain.h"
#include "VulkanDevice.h"

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, VulkanSwapchain* swapchain)
{
	m_device = device;

	//TODO: abstract this aways if possible
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchain->GetImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	ValidCheck(vkCreateRenderPass(m_device->GetDevice(), &renderPassInfo, nullptr, &m_renderPass));
}

VulkanRenderPass::~VulkanRenderPass()
{
	if (m_renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(m_device->GetDevice(), m_renderPass, nullptr);
	}
}

VkRenderPass VulkanRenderPass::GetRenderPass()
{
	return m_renderPass;
}
