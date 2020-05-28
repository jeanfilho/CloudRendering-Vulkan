#include "stdafx.h"
#include "RenderTechniquePPB.h"

RenderTechniquePPB::RenderTechniquePPB(VulkanDevice* device, PushConstants* pushConstants, float initialRadius) : RenderTechnique(device, pushConstants), m_initialRadius(initialRadius)
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
		utilities::ReadFile("../shaders/PPM_CalculateAABB.comp.spv", fittingSPV);
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
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
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
		utilities::ReadFile("../shaders/PPM_GenerateHierarchy.comp.spv", hierarchySPV);
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
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
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
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t))
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
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t))
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
			initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t))
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
	//TODO
}