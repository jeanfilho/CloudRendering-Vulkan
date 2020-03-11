#pragma once

class VulkanDevice;
class VulkanPipelineLayout;
class VulkanShaderModule;

class VulkanComputePipeline
{
public:
	VulkanComputePipeline(VulkanDevice* device, VulkanPipelineLayout* layout, VulkanShaderModule* shaderModule);
	~VulkanComputePipeline();

	VkPipeline GetPipeline();

private:
	VulkanDevice* m_device = nullptr;
	VkPipeline m_computePipeline = VK_NULL_HANDLE;
};
