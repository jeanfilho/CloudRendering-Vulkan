#pragma once

#include "RenderTechnique.h"

class RenderTechniquePT : public RenderTechnique
{
public:
	RenderTechniquePT(VulkanDevice* device, const CameraProperties* cameraProperties);
	~RenderTechniquePT();


	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeBufferInfo, unsigned int frameNr) override;

	virtual uint32_t GetRequiredSetCount() const override;
	virtual void GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const override;	

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, VkImage swapchainImage, VkFormat swapchainImageFormat) override;

private:
	VulkanShaderModule* m_shader = nullptr;
};