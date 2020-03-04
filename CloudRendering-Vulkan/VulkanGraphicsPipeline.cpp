#include "stdafx.h"
#include "VulkanGraphicsPipeline.h"

#include "VulkanDevice.h"
#include "VulkanPipelineLayout.h"
#include "VulkanRenderPass.h"
#include "VulkanShaderModule.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, VulkanPipelineLayout* pipelineLayout, VulkanRenderPass* renderPass, std::vector<VulkanShaderModule*>& shaderModules)
{
	m_device = device;

	// Position 0 is hardcoded for vertex, position 1 is hard coded for fragment
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = initializers::PipelineShaderStageCreateInfo(shaderModules[0]->GetShaderModule(), VK_SHADER_STAGE_VERTEX_BIT);
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = initializers::PipelineShaderStageCreateInfo(shaderModules[1]->GetShaderModule(), VK_SHADER_STAGE_FRAGMENT_BIT);
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &pipelineLayout->GetVertexInputInfo();
	pipelineInfo.pInputAssemblyState = &pipelineLayout->GetInputAssembly();
	pipelineInfo.pViewportState = &pipelineLayout->GetViewportState();
	pipelineInfo.pRasterizationState = &pipelineLayout->GetRasterizer();
	pipelineInfo.pMultisampleState = &pipelineLayout->GetMultisampling();
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &pipelineLayout->GetColorBlending();
	pipelineInfo.pDynamicState = nullptr; // Optional

	pipelineInfo.layout = pipelineLayout->GetPipelineLayout();

	pipelineInfo.renderPass = renderPass->GetRenderPass();
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	ValidCheck(vkCreateGraphicsPipelines(m_device->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	if (m_graphicsPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device->GetDevice(), m_graphicsPipeline, nullptr);
	}
}
