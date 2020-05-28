#pragma once

struct CameraProperties;
class VulkanDescriptorPool;

#include "VulkanDevice.h"
#include "VulkanPipelineLayout.h"
#include "VulkanComputePipeline.h"
#include "VulkanDevice.h"
#include "VulkanShaderModule.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanDescriptorSetLayout.h"

class RenderTechnique
{
	friend VulkanDescriptorPool;

public:
	RenderTechnique(VulkanDevice* device, PushConstants* pushConstants);
	~RenderTechnique();

	void UpdateDescriptorSets();
	
	virtual void GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const = 0;
	virtual void SetFrameResources(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain) = 0;
	virtual void ClearFrameResources() = 0;

	virtual uint32_t GetRequiredSetCount() const = 0;
	virtual void GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const = 0;

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr) = 0;

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex) = 0;

protected:
	VulkanDevice* m_device = nullptr;
	PushConstants* m_pushConstants = nullptr;
	std::vector<VkWriteDescriptorSet> m_writeQueue;
	std::vector<VkDescriptorSet> m_descriptorSets;
};