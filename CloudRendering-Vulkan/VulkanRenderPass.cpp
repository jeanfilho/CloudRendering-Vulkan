#include "stdafx.h"
#include "VulkanRenderPass.h"

#include "VulkanSwapchain.h"
#include "VulkanDevice.h"

VulkanRenderPass::VulkanRenderPass()
{
}

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, VulkanSwapchain* swapchain)
{
	m_device = device;

	AllocateResources(swapchain);
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

void VulkanRenderPass::AllocateResources(VulkanSwapchain* swapchain)
{
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


	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	ValidCheck(vkCreateRenderPass(m_device->GetDevice(), &renderPassInfo, nullptr, &m_renderPass));
}
