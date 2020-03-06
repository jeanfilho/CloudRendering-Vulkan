#pragma once

class VulkanDevice;
class VulkanPipelineLayout;
class VulkanRenderPass;
class VulkanShaderModule;
class VulkanSwapchain;

class VulkanGraphicsPipeline
{
public:
	VulkanGraphicsPipeline(VulkanDevice* device, VulkanSwapchain* swapchain, VulkanPipelineLayout* pipelineLayout, VulkanRenderPass* renderPass, std::vector<VulkanShaderModule*>& shaderModules);
	~VulkanGraphicsPipeline();

	VkPipeline GetPipeline();

private:
	VulkanDevice* m_device = nullptr;
	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
};