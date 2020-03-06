#include "stdafx.h"
#include "VulkanPipelineLayout.h"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice* device, VulkanSwapchain* swapchain)
{
	m_device = device;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(m_device->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	if (m_pipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(m_device->GetDevice(), m_pipelineLayout, nullptr);
	}
}

VkPipelineLayout& VulkanPipelineLayout::GetPipelineLayout()
{
	return m_pipelineLayout;
}
