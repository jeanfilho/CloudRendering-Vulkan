#include "stdafx.h"
#include "VulkanPipelineLayout.h"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice* device, std::vector<VkDescriptorSetLayout>& setLayouts, std::vector<VkPushConstantRange>& pushConstantRanges)
{
	m_device = device;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	ValidCheck(vkCreatePipelineLayout(m_device->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
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
