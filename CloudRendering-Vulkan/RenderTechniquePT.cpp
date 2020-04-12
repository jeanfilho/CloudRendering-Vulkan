#include "stdafx.h"
#include "RenderTechniquePT.h"

#include "VulkanPipelineLayout.h"
#include "VulkanComputePipeline.h"
#include "VulkanShaderModule.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanDescriptorSetLayout.h"

RenderTechniquePT::RenderTechniquePT(VulkanDevice* device, const CameraProperties* cameraProperties) : RenderTechnique(device, cameraProperties)
{
	// Create Shader
	std::vector<char> pathTracerSPV;
	utilities::ReadFile("../shaders/PathTracer.comp.spv", pathTracerSPV);
	m_shader = new VulkanShaderModule(m_device, pathTracerSPV);

	// Path Tracer Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> pathTracerSetLayoutBindings = {
		// Binding 0: Output 2D image (write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Camera properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Cloud grid 3D sampler (read)
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 4: Parameters (read)
		initializers::DescriptorSetLayoutBinding(4, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 5: Shadow volume 3D sampler (read)
		initializers::DescriptorSetLayoutBinding(5, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 5: Shadow volume properties (read)
		initializers::DescriptorSetLayoutBinding(6, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	};
	m_descriptorSetLayout = new VulkanDescriptorSetLayout(m_device, pathTracerSetLayoutBindings);

	// Path tracer pipeline;
	std::vector<VkPushConstantRange> ptPushConstantRanges
	{
		initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
	};
	std::vector<VkDescriptorSetLayout> ptSetLayouts
	{
		m_descriptorSetLayout->GetLayout()
	};

	m_pipelineLayout = new VulkanPipelineLayout(m_device, ptSetLayouts, ptPushConstantRanges);
	m_pipeline = new VulkanComputePipeline(m_device, m_pipelineLayout, m_shader);
}

RenderTechniquePT::~RenderTechniquePT()
{
	delete m_shader;
	delete m_descriptorSetLayout;
	delete m_pipeline;
	delete m_pipelineLayout;
}


void RenderTechniquePT::QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo));
}

void RenderTechniquePT::QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &cloudImageInfo));
}

void RenderTechniquePT::QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &cameraBufferInfo));

}

void RenderTechniquePT::QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &parametersBufferInfo));

}

void RenderTechniquePT::QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6, &shadowVolumeBufferInfo));

}

void RenderTechniquePT::QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeBufferInfo, unsigned int frameNr)
{
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &shadowVolumeBufferInfo));

}

uint32_t RenderTechniquePT::GetRequiredSetCount() const
{
	return 1;
}

void RenderTechniquePT::GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& poolSizes) const
{
	poolSizes.push_back(initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3));		// Cloud Properties + Camera Properties + Parameters
	poolSizes.push_back(initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1));// Cloud grid sampler3D * Frames in flight
	poolSizes.push_back(initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1));			// Render image
}

void RenderTechniquePT::RecordDrawCommands(VkCommandBuffer commandBuffer, VkImage swapchainImage, VkFormat swapchainImageFormat)
{
	// Change Result Image Layout to writeable
	utilities::CmdTransitionImageLayout(commandBuffer, m_image->GetImage(), m_image->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// Bind compute pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline->GetPipeline());

	// Bind descriptor set (resources)
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout->GetPipelineLayout(), 0, static_cast<uint32_t>(m_descriptorSets.size()), m_descriptorSets.data(), 0, nullptr);

	// Start compute shader
	vkCmdDispatch(commandBuffer, m_cameraProperties->GetWidth(), m_cameraProperties->GetHeight(), 1);

	// Change swapchain image layout to dst blit
	utilities::CmdTransitionImageLayout(commandBuffer, swapchainImage, swapchainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Change result image layout to scr blit
	utilities::CmdTransitionImageLayout(commandBuffer, m_image->GetImage(), m_image->GetFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// Copy result to swapchain image
	VkImageSubresourceLayers layers{};
	layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layers.layerCount = 1;
	layers.mipLevel = 0;

	VkExtent3D extents = m_image->GetExtent();
	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0,0,0 };
	blit.srcOffsets[1] = { static_cast<int32_t>(extents.width), static_cast<int32_t>(extents.height), static_cast<int32_t>(extents.depth) };
	blit.srcSubresource = layers;
	blit.dstOffsets[0] = { 0,0,0 };
	blit.dstOffsets[1] = { m_cameraProperties->GetWidth(), m_cameraProperties->GetHeight(), 1 };
	blit.dstSubresource = layers;

	vkCmdBlitImage(commandBuffer, m_image->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
}
