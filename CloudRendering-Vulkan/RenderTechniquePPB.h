#pragma once

#include "RenderTechnique.h"

class VulkanBuffer;

class RenderTechniquePPB : public RenderTechnique
{
public:
	RenderTechniquePPB(VulkanDevice* device, PushConstants* pushConstants, CameraProperties* cameraProperties, float initialRadius);
	~RenderTechniquePPB();

	void AllocateResources();
	void FreeResources();

	void UpdatePhotonMapProperties(VulkanBuffer* photonMapPropertiesBuffer, unsigned int frameNr);

	virtual void GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const override;
	virtual void SetFrameReferences(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain) override;
	virtual void ClearFrameReferences() override;

	virtual uint32_t GetRequiredSetCount() const override;
	virtual void GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const override;

	virtual void QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr) override;
	virtual void QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr) override;
	virtual void QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr) override;

	virtual void RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex) override;

	void GetDebug();

private:
	void UpdateRadius(unsigned int frameNumber);

private:

	// Auxiliary structures and enums
	struct LBVHPushConstants
	{
		uint32_t baseShift = 0;
		uint32_t currentBuffer = 0;
		uint32_t workGroupCount = 0;

	} m_lbvhPushConstants;

	/*const enum EResourceOffsets
	{
		Tracing = 0,
		LocalSort = Tracing + 6,
		PrefixSum = LocalSort + 2,
		GlobalSort = PrefixSum + 2,
		Hierarchy = GlobalSort + 3,
		Fitting = Hierarchy + 2,
		Estimate = Fitting + 2
	};*/

	const enum ESetIndex
	{
		ESetIndex_Tracing = 0,
		ESetIndex_LocalSort,
		ESetIndex_PrefixSum,
		ESetIndex_GlobalSort,
		ESetIndex_Hierarchy,
		ESetIndex_Fitting,
		ESetIndex_Estimate,
		ESetIndex_SetCount
	};

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
	VulkanShaderModule* m_localSortShader = nullptr;
	VulkanDescriptorSetLayout* m_localSortDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_localSortPipelineLayout = nullptr;
	VulkanComputePipeline* m_localSortPipeline = nullptr;

	VulkanShaderModule* m_prefixSumShader = nullptr;
	VulkanDescriptorSetLayout* m_prefixSumDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_prefixSumPipelineLayout = nullptr;
	VulkanComputePipeline* m_prefixSumPipeline = nullptr;

	VulkanShaderModule* m_globalSortShader = nullptr;
	VulkanDescriptorSetLayout* m_globalSortDescriptorSetLayout = nullptr;
	VulkanPipelineLayout* m_globalSortPipelineLayout = nullptr;
	VulkanComputePipeline* m_globalSortPipeline = nullptr;

	// GPU Data
	VulkanBuffer* m_photonBeams = nullptr;
	VulkanBuffer* m_photonBeamsData = nullptr;
	VulkanBuffer* m_lbvh = nullptr;
	VulkanBuffer* m_localHistogram = nullptr;
	VulkanBuffer* m_scannedHistogram = nullptr;

	// References and Parameters
	const CameraProperties* m_cameraProperties = nullptr;
	const PhotonMapProperties* m_photonMapProperties = nullptr;
	VulkanSwapchain* m_swapchain = nullptr;

	std::vector<VulkanImage*> m_images;
	std::vector<VulkanImageView*> m_imageViews;

	const float m_initialRadius = 0;
	const float m_alpha = .8f;

	const size_t m_maxBeamCount = 4096;
	const unsigned int m_workgroupsPerPass = 28;
	const unsigned int m_beamsPerWorkgroup = 2;
	const unsigned int m_beamsPerPass = m_workgroupsPerPass * m_beamsPerWorkgroup;


	//TEMP DEBUG
	struct PhotonBeams
	{
		glm::uvec4 stuff;
		PhotonBeam beams[4096];
	} m_debugBeams;
};