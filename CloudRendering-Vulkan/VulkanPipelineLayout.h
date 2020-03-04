#pragma once

// Fwd Decl.
class VulkanDevice;
class VulkanSwapchain;

class VulkanPipelineLayout
{
public:
	VulkanPipelineLayout(VulkanDevice* device, VulkanSwapchain* swapchain);
	~VulkanPipelineLayout();

	VkPipelineLayout& GetPipelineLayout();

	VkPipelineDynamicStateCreateInfo& GetDynamicState();
	VkPipelineVertexInputStateCreateInfo& GetVertexInputInfo();
	VkPipelineInputAssemblyStateCreateInfo& GetInputAssembly();
	VkPipelineViewportStateCreateInfo& GetViewportState();
	VkPipelineRasterizationStateCreateInfo& GetRasterizer();
	VkPipelineMultisampleStateCreateInfo& GetMultisampling();
	VkPipelineColorBlendStateCreateInfo& GetColorBlending();

private:
	VulkanDevice* m_device = nullptr;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkViewport m_viewport{};
	VkRect2D m_scissor{};
	VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
	std::vector<VkDynamicState> m_dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT,	VK_DYNAMIC_STATE_LINE_WIDTH };

	VkPipelineDynamicStateCreateInfo m_dynamicState{};
	VkPipelineVertexInputStateCreateInfo m_vertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo m_inputAssembly{};
	VkPipelineViewportStateCreateInfo m_viewportState{};
	VkPipelineRasterizationStateCreateInfo m_rasterizer{};
	VkPipelineMultisampleStateCreateInfo m_multisampling{};
	VkPipelineColorBlendStateCreateInfo m_colorBlending{};
};