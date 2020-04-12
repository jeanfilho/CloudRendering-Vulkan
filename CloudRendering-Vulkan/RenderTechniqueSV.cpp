#include "stdafx.h"
#include "RenderTechniqueSV.h"

RenderTechniqueSV::RenderTechniqueSV(VulkanDevice* device, const ShadowVolumeProperties* shadowVolumeProperties) : RenderTechnique(device), m_shadowVolumeProperties(shadowVolumeProperties)
{
	// Shader Modules
	std::vector<char> shadowVolumeSPV;
	utilities::ReadFile("../shaders/ShadowVolume.comp.spv", shadowVolumeSPV);
	m_shader = new VulkanShaderModule(m_device, shadowVolumeSPV);


	// Shadow Volume Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> shadowVolumeLayoutBindings = {
		// Binding 0: Output 3D image (write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Shadow volume properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Cloud grid texel buffer (read)
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	};
	m_descriptorSetLayout = new VulkanDescriptorSetLayout(m_device, shadowVolumeLayoutBindings);


	// Shadow volume pipeline;
	std::vector<VkPushConstantRange> svPushConstantRanges;
	std::vector<VkDescriptorSetLayout> svSetLayouts{ m_descriptorSetLayout->GetLayout() };
	m_pipelineLayout = new VulkanPipelineLayout(m_device, svSetLayouts, svPushConstantRanges);
	m_pipeline = new VulkanComputePipeline(m_device, m_pipelineLayout, m_shader);
}

RenderTechniqueSV::~RenderTechniqueSV()
{
	delete m_shader;
	delete m_descriptorSetLayout;
	delete m_pipelineLayout;
	delete m_pipeline;
}

void RenderTechniqueSV::SetFrameResources(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain)
{
	m_image = frameImages[0];
	m_imageView = frameImageViews[0];

	// Update compute bindings for output image
	std::vector<VkWriteDescriptorSet> writes;
	for (size_t i = 0; i < m_descriptorSets.size(); i++)
	{
		auto imageInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, m_imageView->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &imageInfo));
	};
	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void RenderTechniqueSV::ClearFrameResources()
{
}

void RenderTechniqueSV::QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo));
}

void RenderTechniqueSV::QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &cloudImageInfo));
}

void RenderTechniqueSV::QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr)
{
}

void RenderTechniqueSV::QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr)
{
}

void RenderTechniqueSV::QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &shadowVolumeBufferInfo));
}

void RenderTechniqueSV::QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &shadowVolumeImageInfo));
}

uint32_t RenderTechniqueSV::GetRequiredSetCount() const
{
	return 1;
}

void RenderTechniqueSV::GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const
{
}

void RenderTechniqueSV::RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex)
{
	utilities::CmdTransitionImageLayout(commandBuffer, m_image->GetImage(), m_image->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline->GetPipeline());

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout->GetPipelineLayout(), 0, static_cast<uint32_t>(m_descriptorSets.size()), m_descriptorSets.data(), 0, nullptr);

	vkCmdDispatch(commandBuffer, m_shadowVolumeProperties->voxelAxisCount, m_shadowVolumeProperties->voxelAxisCount, 1);

	utilities::CmdTransitionImageLayout(commandBuffer, m_image->GetImage(), m_image->GetFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
