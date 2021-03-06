#pragma once

#include "RenderTechnique.h"

class VulkanSampler;

class RenderTechniqueSV : public RenderTechnique
{
public:
	RenderTechniqueSV(VulkanDevice* device, const ShadowVolumeProperties* shadowVolumeProperties, PushConstants* pushConstants);
	~RenderTechniqueSV();

	virtual void GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const override;
	virtual void SetFrameReferences(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain) override;
	virtual void ClearFrameReferences() override;

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeBufferInfo, unsigned int frameNr) override;

	virtual uint32_t GetRequiredSetCount() const override;
	virtual void GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const override;

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex) override;

private:
	VulkanShaderModule* m_shader = nullptr;
	VulkanDescriptorSetLayout* m_descriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_pipelineLayout = nullptr;
	VulkanComputePipeline* m_pipeline = nullptr;

	const ShadowVolumeProperties* m_shadowVolumeProperties = nullptr;

	VulkanSampler* m_cloudSampler = nullptr;
	VulkanImage* m_image = nullptr;
	VulkanImageView* m_imageView = nullptr;
};