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
	RenderTechnique(VulkanDevice* device, const CameraProperties* cameraProperties) : m_device(device), m_cameraProperties(cameraProperties)
	{
	}
	~RenderTechnique()
	{
		ClearFrameResources();
	}
	void CreateFrameResources();
	void ClearFrameResources();

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, VkImage swapchainImage, VkFormat swapchainImageFormat) = 0;

	VkPipelineLayout GetPipelineLayout() const;
	void GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const;

	void UpdateDescriptorSets();

	virtual uint32_t GetRequiredSetCount() const = 0;
	virtual void GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const = 0;

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr) = 0;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeBufferInfo, unsigned int frameNr) = 0;

protected:
	VulkanDevice* m_device = nullptr;
	const CameraProperties* m_cameraProperties = nullptr;
	std::vector<VkWriteDescriptorSet> m_writeQueue;

	VulkanDescriptorSetLayout* m_descriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_pipelineLayout = nullptr;
	VulkanComputePipeline* m_pipeline = nullptr;
	VulkanImage* m_image = nullptr;
	VulkanImageView* m_imageView = nullptr;
	std::vector<VkDescriptorSet> m_descriptorSets;
};