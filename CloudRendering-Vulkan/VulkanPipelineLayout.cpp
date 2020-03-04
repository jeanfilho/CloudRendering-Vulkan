#include "stdafx.h"
#include "VulkanPipelineLayout.h"

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice* device, VulkanSwapchain* swapchain)
{
	m_device = device;

	//TODO this is hard coded from the tutorial - will probably change afterwards
	m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_vertexInputInfo.vertexBindingDescriptionCount = 0;
	m_vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	m_vertexInputInfo.vertexAttributeDescriptionCount = 0;
	m_vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	m_inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_inputAssembly.primitiveRestartEnable = VK_FALSE;

	m_viewport.x = 0.0f;
	m_viewport.y = 0.0f;
	m_viewport.width = (float)swapchain->GetExtent().width;
	m_viewport.height = (float)swapchain->GetExtent().height;
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	m_scissor.offset = { 0, 0 };
	m_scissor.extent = swapchain->GetExtent();

	m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewportState.viewportCount = 1;
	m_viewportState.pViewports = &m_viewport;
	m_viewportState.scissorCount = 1;
	m_viewportState.pScissors = &m_scissor;

	m_rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterizer.depthClampEnable = VK_FALSE;
	m_rasterizer.rasterizerDiscardEnable = VK_FALSE;
	m_rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	m_rasterizer.lineWidth = 1.0f;
	m_rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	m_rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_rasterizer.depthBiasEnable = VK_FALSE;
	m_rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	m_rasterizer.depthBiasClamp = 0.0f; // Optional
	m_rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	m_multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisampling.sampleShadingEnable = VK_FALSE;
	m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_multisampling.minSampleShading = 1.0f; // Optional
	m_multisampling.pSampleMask = nullptr; // Optional
	m_multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	m_multisampling.alphaToOneEnable = VK_FALSE; // Optional

	m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_colorBlendAttachment.blendEnable = VK_FALSE;
	m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	m_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	m_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	m_colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_colorBlending.logicOpEnable = VK_FALSE;
	m_colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	m_colorBlending.attachmentCount = 1;
	m_colorBlending.pAttachments = &m_colorBlendAttachment;
	m_colorBlending.blendConstants[0] = 0.0f; // Optional
	m_colorBlending.blendConstants[1] = 0.0f; // Optional
	m_colorBlending.blendConstants[2] = 0.0f; // Optional
	m_colorBlending.blendConstants[3] = 0.0f; // Optional

	m_dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	m_dynamicState.dynamicStateCount = 2;
	m_dynamicState.pDynamicStates = m_dynamicStates.data();

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

VkPipelineDynamicStateCreateInfo& VulkanPipelineLayout::GetDynamicState()
{
	return m_dynamicState;
}

VkPipelineVertexInputStateCreateInfo& VulkanPipelineLayout::GetVertexInputInfo()
{
	return m_vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo& VulkanPipelineLayout::GetInputAssembly()
{
	return m_inputAssembly;
}

VkPipelineViewportStateCreateInfo& VulkanPipelineLayout::GetViewportState()
{
	return m_viewportState;
}

VkPipelineRasterizationStateCreateInfo& VulkanPipelineLayout::GetRasterizer()
{
	return m_rasterizer;
}

VkPipelineMultisampleStateCreateInfo& VulkanPipelineLayout::GetMultisampling()
{
	return m_multisampling;
}

VkPipelineColorBlendStateCreateInfo& VulkanPipelineLayout::GetColorBlending()
{
	return m_colorBlending;
}
