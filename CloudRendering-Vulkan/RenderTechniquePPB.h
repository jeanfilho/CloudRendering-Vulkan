#pragma once

#include "RenderTechnique.h"

class RenderTechniquePPB : public RenderTechnique
{
	RenderTechniquePPB(VulkanDevice* device, PushConstants* pushConstants);
	~RenderTechniquePPB();

	virtual void GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const override;
	virtual void SetFrameResources(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain) override;
	virtual void ClearFrameResources() override;

	virtual uint32_t GetRequiredSetCount() const override;
	virtual void GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const override;

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr) override;

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex) override;

private:
	// Photon Beams
	VulkanShaderModule* m_estimateShader = nullptr;
	VulkanDescriptorSetLayout* m_estimateDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_estimatePipelineLayout = nullptr;
	VulkanComputePipeline* m_estimatePipeline = nullptr;

	VulkanShaderModule* m_tracingShader = nullptr;
	VulkanDescriptorSetLayout* m_tracingDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_tracingPipelineLayout = nullptr;
	VulkanComputePipeline* m_tracingPipeline = nullptr;

	// LBVH
	VulkanShaderModule* m_fittingShader = nullptr;
	VulkanDescriptorSetLayout* m_fittingDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_fittingPipelineLayout = nullptr;
	VulkanComputePipeline* m_fittingPipeline = nullptr;

	VulkanShaderModule* m_hierarchyShader = nullptr;
	VulkanDescriptorSetLayout* m_hierarchyDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_hierarchyPipelineLayout = nullptr;
	VulkanComputePipeline* m_hierarchyPipeline = nullptr;

	// Radix Sort
	VulkanShaderModule* m_globalSortShader = nullptr;
	VulkanDescriptorSetLayout* m_globalSortDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_globalSortPipelineLayout = nullptr;
	VulkanComputePipeline* m_globalSortPipeline = nullptr;

	VulkanShaderModule* m_localSortShader = nullptr;
	VulkanDescriptorSetLayout* m_localSortDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_localSortPipelineLayout = nullptr;
	VulkanComputePipeline* m_localSortPipeline = nullptr;

	VulkanShaderModule* m_prefixSumShader = nullptr;
	VulkanDescriptorSetLayout* m_prefixSumDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_prefixSumPipelineLayout = nullptr;
	VulkanComputePipeline* m_prefixSumPipeline = nullptr;

	// GPU Data
	VulkanBuffer* m_photonBeams = nullptr;
	VulkanBuffer* m_sortedPhotonBeams = nullptr;
	VulkanBuffer* m_collisionMap = nullptr;

	// References and Parameters
	const CameraProperties* m_cameraProperties = nullptr;
	const PhotonMapProperties* m_photonMapProperties = nullptr;
	VulkanSwapchain* m_swapchain = nullptr;

	std::vector<VulkanImage*> m_images;
	std::vector<VulkanImageView*> m_imageViews;

	const float m_initialRadius override;
	const float m_alpha = .8f;
};