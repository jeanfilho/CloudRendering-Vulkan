#include "stdafx.h"
#include "RenderTechniquePPB.h"

#include "VulkanBuffer.h"
#include "VulkanSwapchain.h"

RenderTechniquePPB::RenderTechniquePPB(VulkanDevice* device, PushConstants* pushConstants, CameraProperties* cameraProperties, float initialRadius) : RenderTechnique(device, pushConstants), m_initialRadius(initialRadius), m_cameraProperties(cameraProperties)
{
	// Photon Tracer
	{
		std::vector<char> photonTracerSPV;
		utilities::ReadFile("../shaders/PPB_PT.comp.spv", photonTracerSPV);
		m_tracingShader = new VulkanShaderModule(m_device, photonTracerSPV);

		// Photon Tracer Tracer Descriptor Set Layout
		std::vector<VkDescriptorSetLayoutBinding> tracingSetLayoutBindings = {
			// Binding 0: Photon Beams
			initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Photon Beams Shared Data
			initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 2: Photon Beams Properties
			initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
			// Binding 3: Cloud Sampler
			initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
			// Binding 4: Cloud Properties
			initializers::DescriptorSetLayoutBinding(4, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
			// Binding 5: Parameters (read)
			initializers::DescriptorSetLayoutBinding(5, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		};
		m_tracingDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, tracingSetLayoutBindings);

		std::vector<VkPushConstantRange> tracingPushConstantRanges
		{
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
		};
		std::vector<VkDescriptorSetLayout> tracingSetLayouts
		{
			m_tracingDescriptorSetLayout->GetLayout()
		};

		m_tracingPipelineLayout = new VulkanPipelineLayout(m_device, tracingSetLayouts, tracingPushConstantRanges);
		m_tracingPipeline = new VulkanComputePipeline(m_device, m_tracingPipelineLayout, m_tracingShader);
	}

	// Photon Estimate
	{
		std::vector<char> photonEstimatorSPV;
		utilities::ReadFile("../shaders/PPB_PE.comp.spv", photonEstimatorSPV);
		m_estimateShader = new VulkanShaderModule(m_device, photonEstimatorSPV);

		std::vector<VkDescriptorSetLayoutBinding> estimateSetLayoutBindings = {
			// Binding 0: Result Image
			initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
			// Binding 1: Camera Properties
			initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
			// Binding 2: Sorted Photon Beams
			initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 3: Photon Beams Data
			initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 4: Tree
			initializers::DescriptorSetLayoutBinding(4, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 5: Cloud Sampler
			initializers::DescriptorSetLayoutBinding(5, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
			// Binding 6: Cloud Properties
			initializers::DescriptorSetLayoutBinding(6, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
			// Binding 7: Parameters
			initializers::DescriptorSetLayoutBinding(7, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
			// Binding 8: Shadow Volume Sampler
			initializers::DescriptorSetLayoutBinding(8, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
			// Binding 9: Shadow Volume Properties
			initializers::DescriptorSetLayoutBinding(9, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		};
		m_estimateDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, estimateSetLayoutBindings);

		std::vector<VkPushConstantRange> estimatePushConstantRanges
		{
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
		};
		std::vector<VkDescriptorSetLayout> estimateSetLayouts
		{
			m_estimateDescriptorSetLayout->GetLayout()
		};

		m_estimatePipelineLayout = new VulkanPipelineLayout(m_device, estimateSetLayouts, estimatePushConstantRanges);
		m_estimatePipeline = new VulkanComputePipeline(m_device, m_estimatePipelineLayout, m_estimateShader);
	}

	// AABB Fitting
	{
		std::vector<char> fittingSPV;
		utilities::ReadFile("../shaders/PPB_CalculateAABB.comp.spv", fittingSPV);
		m_fittingShader = new VulkanShaderModule(m_device, fittingSPV);

		std::vector<VkDescriptorSetLayoutBinding> fittingSetLayoutBindings = {
			// Binding 0: Sorted Photon Beams
			initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Tree
			initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		};
		m_fittingDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, fittingSetLayoutBindings);

		std::vector<VkPushConstantRange> fittingPushConstantRanges
		{
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LBVHPushConstants))
		};
		std::vector<VkDescriptorSetLayout> fittingSetLayouts
		{
			m_fittingDescriptorSetLayout->GetLayout()
		};

		m_fittingPipelineLayout = new VulkanPipelineLayout(m_device, fittingSetLayouts, fittingPushConstantRanges);
		m_fittingPipeline = new VulkanComputePipeline(m_device, m_fittingPipelineLayout, m_fittingShader);
	}

	// Hierarchy Generation
	{
		std::vector<char> hierarchySPV;
		utilities::ReadFile("../shaders/PPB_GenerateHierarchy.comp.spv", hierarchySPV);
		m_hierarchyShader = new VulkanShaderModule(m_device, hierarchySPV);

		std::vector<VkDescriptorSetLayoutBinding> hierarchySetLayoutBindings = {
			// Binding 0: Sorted Photon Beams
			initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Tree
			initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		};
		m_hierarchyDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, hierarchySetLayoutBindings);

		std::vector<VkPushConstantRange> hierarchyPushConstantRanges
		{
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LBVHPushConstants))
		};
		std::vector<VkDescriptorSetLayout> hierarchySetLayouts
		{
			m_hierarchyDescriptorSetLayout->GetLayout()
		};

		m_hierarchyPipelineLayout = new VulkanPipelineLayout(m_device, hierarchySetLayouts, hierarchyPushConstantRanges);
		m_hierarchyPipeline = new VulkanComputePipeline(m_device, m_hierarchyPipelineLayout, m_hierarchyShader);
	}

	// Radix Local Sort
	{
		std::vector<char> localSortSPV;
		utilities::ReadFile("../shaders/PPB_RadixSort_LocalSort.comp.spv", localSortSPV);
		m_localSortShader = new VulkanShaderModule(m_device, localSortSPV);

		std::vector<VkDescriptorSetLayoutBinding> localSortSetLayoutBindings = {
			// Binding 0: Photon Beams
			initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Sorted Photon Beams
			initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Histogram 4b
			initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		};
		m_localSortDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, localSortSetLayoutBindings);

		std::vector<VkPushConstantRange> localSortPushConstantRanges
		{
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LBVHPushConstants))
		};
		std::vector<VkDescriptorSetLayout> localSortSetLayouts
		{
			m_localSortDescriptorSetLayout->GetLayout()
		};

		m_localSortPipelineLayout = new VulkanPipelineLayout(m_device, localSortSetLayouts, localSortPushConstantRanges);
		m_localSortPipeline = new VulkanComputePipeline(m_device, m_localSortPipelineLayout, m_localSortShader);
	}

	// Radix Global Sort
	{
		std::vector<char> globalSortSPV;
		utilities::ReadFile("../shaders/PPB_RadixSort_GlobalSort.comp.spv", globalSortSPV);
		m_globalSortShader = new VulkanShaderModule(m_device, globalSortSPV);

		std::vector<VkDescriptorSetLayoutBinding> globalSortSetLayoutBindings = {
			// Binding 0: Sorted Photon Beams
			initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Global Sorted Photon Beams
			initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Histogram 4b
			initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Scanned Histogram 4b
			initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		};
		m_globalSortDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, globalSortSetLayoutBindings);

		std::vector<VkPushConstantRange> globalSortPushConstantRanges
		{
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LBVHPushConstants))
		};
		std::vector<VkDescriptorSetLayout> globalSortSetLayouts
		{
			m_globalSortDescriptorSetLayout->GetLayout()
		};

		m_globalSortPipelineLayout = new VulkanPipelineLayout(m_device, globalSortSetLayouts, globalSortPushConstantRanges);
		m_globalSortPipeline = new VulkanComputePipeline(m_device, m_globalSortPipelineLayout, m_globalSortShader);
	}

	// Radix Prefix Sum
	{
		std::vector<char> prefixSumSPV;
		utilities::ReadFile("../shaders/PPB_RadixSort_PrefixSum.comp.spv", prefixSumSPV);
		m_prefixSumShader = new VulkanShaderModule(m_device, prefixSumSPV);

		std::vector<VkDescriptorSetLayoutBinding> prefixSumSetLayoutBindings = {
			// Binding 0: Histogram 4b
			initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
			// Binding 1: Scanned Histogram 4b
			initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		};
		m_prefixSumDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, prefixSumSetLayoutBindings);

		std::vector<VkPushConstantRange> prefixSumPushConstantRanges
		{
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LBVHPushConstants))
		};
		std::vector<VkDescriptorSetLayout> prefixSumSetLayouts
		{
			m_prefixSumDescriptorSetLayout->GetLayout()
		};

		m_prefixSumPipelineLayout = new VulkanPipelineLayout(m_device, prefixSumSetLayouts, prefixSumPushConstantRanges);
		m_prefixSumPipeline = new VulkanComputePipeline(m_device, m_prefixSumPipelineLayout, m_prefixSumShader);
	}
}

RenderTechniquePPB::~RenderTechniquePPB()
{
	// Photon Beams
	delete m_estimateShader;
	delete m_estimateDescriptorSetLayout;
	delete m_estimatePipelineLayout;
	delete m_estimatePipeline;

	delete m_tracingShader;
	delete m_tracingDescriptorSetLayout;
	delete m_tracingPipelineLayout;
	delete m_tracingPipeline;

	// LBVH
	delete m_fittingShader;
	delete m_fittingDescriptorSetLayout;
	delete m_fittingPipelineLayout;
	delete m_fittingPipeline;

	delete m_hierarchyShader;
	delete m_hierarchyDescriptorSetLayout;
	delete m_hierarchyPipelineLayout;
	delete m_hierarchyPipeline;

	// Radix Sort
	delete m_globalSortShader;
	delete m_globalSortDescriptorSetLayout;
	delete m_globalSortPipelineLayout;
	delete m_globalSortPipeline;

	delete m_localSortShader;
	delete m_localSortDescriptorSetLayout;
	delete m_localSortPipelineLayout;
	delete m_localSortPipeline;

	delete m_prefixSumShader;
	delete m_prefixSumDescriptorSetLayout;
	delete m_prefixSumPipelineLayout;
	delete m_prefixSumPipeline;

	FreeResources();
}

void RenderTechniquePPB::AllocateResources()
{
	if (m_photonBeams)
	{
		FreeResources();
	}

	VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	m_photonBeams = new VulkanBuffer(m_device, &m_debugBeams, sizeof(uint32_t), flags, (sizeof(PhotonBeam) / 4) * m_maxBeamCount * 2 + 4);		// + 4 to account for the count variable
	m_photonBeamsData = new VulkanBuffer(m_device, nullptr, sizeof(uint32_t), flags, (sizeof(PhotonBeamData) / 4) * m_maxBeamCount + 4);	// + 4 to account for the count variable
	m_localHistogram = new VulkanBuffer(m_device, nullptr, sizeof(uint32_t), flags, 16);						// 4 bits at a time, 16 buckets
	m_scannedHistogram = new VulkanBuffer(m_device, nullptr, sizeof(uint32_t), flags, 16);						// 4 bits at a time, 16 buckets
	m_lbvh = new VulkanBuffer(m_device, nullptr, sizeof(TreeNode), flags, 2 * m_maxBeamCount);					// Inner nodes + Leaf nodes - Binary Tree + 1 for the count variable

	auto photonBeamsInfo = initializers::DescriptorBufferInfo(m_photonBeams->GetBuffer(), 0, VK_WHOLE_SIZE);
	auto photonBeamsDataInfo = initializers::DescriptorBufferInfo(m_photonBeamsData->GetBuffer(), 0, VK_WHOLE_SIZE);
	auto localHistogramInfo = initializers::DescriptorBufferInfo(m_localHistogram->GetBuffer(), 0, VK_WHOLE_SIZE);
	auto scannedHistogramInfo = initializers::DescriptorBufferInfo(m_scannedHistogram->GetBuffer(), 0, VK_WHOLE_SIZE);
	auto lbvhInfo = initializers::DescriptorBufferInfo(m_lbvh->GetBuffer(), 0, VK_WHOLE_SIZE);

	std::vector<VkWriteDescriptorSet> writes;

	// Tracing
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Tracing], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &photonBeamsInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Tracing], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &photonBeamsDataInfo));

	// Local Sort
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_LocalSort], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &photonBeamsInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_LocalSort], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &localHistogramInfo));

	// Prefix Sum
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_PrefixSum], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &localHistogramInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_PrefixSum], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &scannedHistogramInfo));

	// Global Sort
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_GlobalSort], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &photonBeamsInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_GlobalSort], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &localHistogramInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_GlobalSort], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &scannedHistogramInfo));

	// Hierarchy
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Hierarchy], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &photonBeamsInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Hierarchy], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &lbvhInfo));

	// Fitting
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Fitting], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &photonBeamsInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Fitting], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &lbvhInfo));

	// Tracing
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &photonBeamsInfo));
	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &photonBeamsDataInfo));

	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void RenderTechniquePPB::FreeResources()
{
	if (m_photonBeams)
	{
		delete m_photonBeams;
		m_photonBeams = nullptr;

		delete m_photonBeamsData;
		m_photonBeamsData = nullptr;

		delete m_localHistogram;
		m_localHistogram = nullptr;

		delete m_scannedHistogram;
		m_scannedHistogram = nullptr;

		delete m_lbvh;
		m_lbvh = nullptr;
	}
}

void RenderTechniquePPB::UpdatePhotonMapProperties(VulkanBuffer* photonMapPropertiesBuffer, unsigned int frameNr)
{
	auto photonMapPropertiesInfo = initializers::DescriptorBufferInfo(photonMapPropertiesBuffer->GetBuffer(), 0, photonMapPropertiesBuffer->GetSize());
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Tracing], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &photonMapPropertiesInfo));

	UpdateDescriptorSets();
}

void RenderTechniquePPB::GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const
{
	outSetLayouts.push_back(m_tracingDescriptorSetLayout->GetLayout());
	outSetLayouts.push_back(m_localSortDescriptorSetLayout->GetLayout());
	outSetLayouts.push_back(m_prefixSumDescriptorSetLayout->GetLayout());
	outSetLayouts.push_back(m_globalSortDescriptorSetLayout->GetLayout());
	outSetLayouts.push_back(m_hierarchyDescriptorSetLayout->GetLayout());
	outSetLayouts.push_back(m_fittingDescriptorSetLayout->GetLayout());
	outSetLayouts.push_back(m_estimateDescriptorSetLayout->GetLayout());
}

void RenderTechniquePPB::SetFrameReferences(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain)
{
	m_images = frameImages;
	m_imageViews = frameImageViews;
	m_swapchain = swapchain;

	// Update compute bindings for output image
	std::vector<VkWriteDescriptorSet> writes;
	auto imageInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, m_imageViews[0]->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);

	writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &imageInfo));

	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void RenderTechniquePPB::ClearFrameReferences()
{
	m_imageViews.clear();
	m_images.clear();
	m_swapchain = nullptr;
}

uint32_t RenderTechniquePPB::GetRequiredSetCount() const
{
	return ESetIndex_SetCount;
}

void RenderTechniquePPB::GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const
{
	outPoolSizes.push_back(initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6)); // Beams * 2 + Beam Data + Histogram + Scanned Histogram + Tree
}

void RenderTechniquePPB::QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Tracing], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &cloudBufferInfo));
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6, &cloudBufferInfo));
}

void RenderTechniquePPB::QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Tracing], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &cloudImageInfo));
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &cloudImageInfo));
}

void RenderTechniquePPB::QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &cameraBufferInfo));
}

void RenderTechniquePPB::QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Tracing], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, &parametersBufferInfo));
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7, &parametersBufferInfo));
}

void RenderTechniquePPB::QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 9, &shadowVolumeBufferInfo));
}

void RenderTechniquePPB::QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[ESetIndex_Estimate], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8, &shadowVolumeImageInfo));
}

void RenderTechniquePPB::RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex)
{
	m_lbvhPushConstants.currentBuffer = 0;
	UpdateRadius(m_pushConstants->frameCount);

	// Photon Tracing
	{
		// Clear previous data
		vkCmdFillBuffer(commandBuffer, m_photonBeams->GetBuffer(), 0, VK_WHOLE_SIZE, 0);
		vkCmdFillBuffer(commandBuffer, m_photonBeamsData->GetBuffer(), 0, VK_WHOLE_SIZE, 0);

		// Wait until buffers are cleared
		VkMemoryBarrier memoryBarrier = initializers::MemBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &memoryBarrier, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE);

		// Push constants
		vkCmdPushConstants(commandBuffer, m_tracingPipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), m_pushConstants);

		// Bind compute pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_tracingPipeline->GetPipeline());

		// Bind descriptor set (resources)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_tracingPipelineLayout->GetPipelineLayout(), 0, 1, m_descriptorSets.data() + ESetIndex_Tracing, 0, nullptr);

		// Start compute shader
		vkCmdDispatch(commandBuffer, m_workgroupsPerPass, 1, 1);
	}

	// Sorting - n passes
	unsigned int passes = 1; // should be size of morton code divided by 4 (e.g. 24 / 4)
	for(unsigned int i = 0; i < passes; i++)
	{
		// Clear previous data
		vkCmdFillBuffer(commandBuffer, m_localHistogram->GetBuffer(), 0, VK_WHOLE_SIZE, 0);
		vkCmdFillBuffer(commandBuffer, m_scannedHistogram->GetBuffer(), 0, VK_WHOLE_SIZE, 0);

		// Wait until tracing is complete to start the estimate
		VkMemoryBarrier memoryBarrier = initializers::MemBarrier(VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &memoryBarrier, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE);

		// Push constants
		m_lbvhPushConstants.baseShift = i * 4;
		vkCmdPushConstants(commandBuffer, m_localSortPipelineLayout->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(LBVHPushConstants), &m_lbvhPushConstants);

		// Bind compute pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_localSortPipeline->GetPipeline());

		// Bind descriptor set (resources)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_localSortPipelineLayout->GetPipelineLayout(), 0, 1, m_descriptorSets.data() + ESetIndex_LocalSort, 0, nullptr);

		// Start compute shader
		vkCmdDispatch(commandBuffer, static_cast<uint32_t>(m_maxBeamCount) / (256 * 4) + 1, 1, 1);

		// Flip buffers
		m_lbvhPushConstants.currentBuffer = (m_lbvhPushConstants.currentBuffer + 1) % 2;
	}

	// Change swapchain image layout to dst blit
	utilities::CmdTransitionImageLayout(commandBuffer, m_swapchain->GetSwapchainImages()[imageIndex], m_swapchain->GetImageFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Change result image layout to scr blit
	utilities::CmdTransitionImageLayout(commandBuffer, m_images[currentFrame]->GetImage(), m_images[currentFrame]->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//utilities::CmdTransitionImageLayout(commandBuffer, m_images[currentFrame]->GetImage(), m_images[currentFrame]->GetFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// Copy result to swapchain image
	VkImageSubresourceLayers layers{};
	layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layers.layerCount = 1;
	layers.mipLevel = 0;

	VkExtent3D extents = m_images[currentFrame]->GetExtent();
	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0,0,0 };
	blit.srcOffsets[1] = { static_cast<int32_t>(extents.width), static_cast<int32_t>(extents.height), static_cast<int32_t>(extents.depth) };
	blit.srcSubresource = layers;
	blit.dstOffsets[0] = { 0,0,0 };
	blit.dstOffsets[1] = { m_cameraProperties->GetWidth(), m_cameraProperties->GetHeight(), 1 };
	blit.dstSubresource = layers;

	vkCmdBlitImage(commandBuffer, m_images[currentFrame]->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_swapchain->GetSwapchainImages()[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
}

void RenderTechniquePPB::GetDebug()
{
	if (m_pushConstants->frameCount > 1)
	{
		vkDeviceWaitIdle(m_device->GetDevice());
		m_photonBeams->GetData();
	}
}

void RenderTechniquePPB::UpdateRadius(unsigned int frameNumber)
{
	if (frameNumber <= 1)
	{
		m_pushConstants->pmRadius = m_initialRadius;
	}
	else
	{
		float sum = 0;
		int constant = (frameNumber - 2) * m_beamsPerPass;
		for (unsigned int j = 1; j <= m_beamsPerPass; j++)
		{
			sum += (constant + j + m_alpha) / (constant + j + 1.0f);
		}
		m_pushConstants->pmRadius = m_pushConstants->pmRadius * sum;
	}
}
