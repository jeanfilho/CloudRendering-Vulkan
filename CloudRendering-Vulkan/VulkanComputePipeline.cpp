#include "stdafx.h"
#include "VulkanComputePipeline.h"

#include "VulkanDevice.h"
#include "VulkanPipelineLayout.h"
#include "VulkanShaderModule.h"

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* device, VulkanPipelineLayout* layout, VulkanShaderModule* shaderModule)
{
	m_device = device;

	VkComputePipelineCreateInfo computeInfo{};
	computeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computeInfo.layout = layout->GetPipelineLayout();
	computeInfo.stage = initializers::PipelineShaderStageCreateInfo(shaderModule->GetShaderModule(), VK_SHADER_STAGE_COMPUTE_BIT);

	ValidCheck(vkCreateComputePipelines(m_device->GetDevice(), VK_NULL_HANDLE, 1, &computeInfo, nullptr, &m_computePipeline));
}

VulkanComputePipeline::~VulkanComputePipeline()
{
	if (m_computePipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device->GetDevice(), m_computePipeline, nullptr);
	}
}

VkPipeline VulkanComputePipeline::GetPipeline()
{
	return m_computePipeline;
}
