#pragma once

#include "RenderTechnique.h"

class VulkanBuffer;
class VulkanBufferView;

class RenderTechniquePPM : public RenderTechnique
{
private:
	const enum ESetIndex
	{
		ESetIndex_Tracing = 0,
		ESetIndex_Estimate,
		ESetIndex_SetCount
	};

public:
	RenderTechniquePPM(VulkanDevice* device, VulkanSwapchain* swapchain, const CameraProperties* cameraProperties, PhotonMapProperties* photonMapProperties, PushConstants* pushConstants, float initialRadius);
	~RenderTechniquePPM();

	void AllocateResources(VulkanBuffer* photonMapPropertiesBuffer);
	void FreeResources();

	virtual void GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const override;

	virtual void SetFrameReferences(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain);
	virtual void ClearFrameReferences();

	virtual uint32_t GetRequiredSetCount() const;

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr);
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr);
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr);
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr);
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr);
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr);

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex);

private:
	void UpdateRadius(unsigned int frameNumber);

private:
	VulkanShaderModule* m_peShader = nullptr;
	VulkanDescriptorSetLayout* m_peDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_pePipelineLayout = nullptr;
	VulkanComputePipeline* m_pePipeline = nullptr;

	VulkanShaderModule* m_ptShader = nullptr;
	VulkanDescriptorSetLayout* m_ptDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_ptPipelineLayout = nullptr;
	VulkanComputePipeline* m_ptPipeline = nullptr;

	VulkanBuffer* m_photonMap = nullptr;
	VulkanBuffer* m_collisionMap = nullptr;

	const CameraProperties* m_cameraProperties = nullptr;
	const PhotonMapProperties* m_photonMapProperties = nullptr;
	VulkanSwapchain* m_swapchain = nullptr;

	std::vector<VulkanImage*> m_images;
	std::vector<VulkanImageView*> m_imageViews;

	const float m_initialRadius = 0;
	const float m_alpha = .8f;
	const uint32_t elementsPerCell = 32;
};