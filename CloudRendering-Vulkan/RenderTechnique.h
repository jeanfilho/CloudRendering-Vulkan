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
	virtual void SetFrameReferences(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain) = 0;
	virtual void ClearFrameReferences() = 0;

	virtual uint32_t GetRequiredSetCount() const = 0;
	inline void GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const
	{
		for (auto& descriptorTypeCount : m_descriptorTypeCountMap)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = descriptorTypeCount.first;
			poolSize.descriptorCount = descriptorTypeCount.second;
            outPoolSizes.push_back(poolSize);
		}
	}

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int imageIdx) = 0;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int imageIdx) = 0;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int imageIdx) = 0;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int imageIdx) = 0;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int imageIdx) = 0;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int imageIdx) = 0;

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int imageIndex) = 0;

protected:
	inline void AddDescriptorTypesCount(std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		for(auto& binding : bindings)
		{
			m_descriptorTypeCountMap[binding.descriptorType] += binding.descriptorCount;
        }
	}

protected:
	VulkanDevice* m_device = nullptr;
	PushConstants* m_pushConstants = nullptr;
	std::vector<VkWriteDescriptorSet> m_writeQueue;
	std::vector<VkDescriptorSet> m_descriptorSets;
	unsigned int m_descriptorSetCount = 0;

	std::unordered_map<VkDescriptorType, unsigned int> m_descriptorTypeCountMap;
};