#pragma once

#include "RenderTechnique.h"

/*
 * Path Tracing render technique
 */
class RenderTechniquePT : public RenderTechnique
{
public:
	RenderTechniquePT(VulkanDevice* device, VulkanSwapchain* swapchain, const CameraProperties* cameraProperties, PushConstants* pushConstants);
	~RenderTechniquePT();


	virtual void GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const override;
	virtual void SetFrameReferences(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain) override;
	virtual void ClearFrameReferences() override;

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr) override;

	virtual uint32_t GetRequiredSetCount() const override;

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex) override;

private:
	VulkanShaderModule* m_shader = nullptr;
	VulkanDescriptorSetLayout* m_descriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_pipelineLayout = nullptr;
	VulkanComputePipeline* m_pipeline = nullptr;

	const CameraProperties* m_cameraProperties = nullptr;
	VulkanSwapchain* m_swapchain = nullptr;
	std::vector<VulkanImage*> m_images;
	std::vector<VulkanImageView*> m_imageViews;
};