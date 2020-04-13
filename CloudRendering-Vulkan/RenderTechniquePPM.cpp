#include "stdafx.h"
#include "RenderTechniquePPM.h"

#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"

RenderTechniquePPM::RenderTechniquePPM(VulkanDevice* device, VulkanSwapchain* swapchain, const CameraProperties* cameraProperties, const PhotonMapProperties* photonMapProperties) :
	RenderTechnique(device),
	m_swapchain(swapchain),
	m_cameraProperties(cameraProperties),
	m_photonMapProperties(photonMapProperties)
{
	// Photon Tracer
	std::vector<char> photonTracerSPV;
	utilities::ReadFile("../shaders/PPM_PT.comp.spv", photonTracerSPV);
	m_ptShader = new VulkanShaderModule(m_device, photonTracerSPV);

	// Photon Tracer Tracer Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> ptSetLayoutBindings = {
		// Binding 0: Output 3D photon map image (read and write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Photon Map Properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Cloud grid 3D sampler (read)
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 4: Parameters (read)
		initializers::DescriptorSetLayoutBinding(4, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 5: Photon collision map (read and write)
		initializers::DescriptorSetLayoutBinding(5, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	};
	m_ptDescriptorSetLayout = new VulkanDescriptorSetLayout(m_device, ptSetLayoutBindings);

	// Path tracer pipeline;
	std::vector<VkPushConstantRange> ptPushConstantRanges
	{
		initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
	};
	std::vector<VkDescriptorSetLayout> ptSetLayouts
	{
		m_ptDescriptorSetLayout->GetLayout()
	};

	m_ptPipelineLayout = new VulkanPipelineLayout(m_device, ptSetLayouts, ptPushConstantRanges);
	m_ptPipeline = new VulkanComputePipeline(m_device, m_ptPipelineLayout, m_ptShader);

	// Photon Estimate
	std::vector<char> photonEstimateSPV;
	utilities::ReadFile("../shaders/PPM_PE.comp.spv", photonEstimateSPV);
	m_shader = new VulkanShaderModule(m_device, photonEstimateSPV);


	// Photon Estimate Descriptor Set Layout
	std::vector<VkDescriptorSetLayoutBinding> peSetLayoutBindings = {
		// Binding 0: Output 2D image (write)
		initializers::DescriptorSetLayoutBinding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 1: Camera properties (read)
		initializers::DescriptorSetLayoutBinding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 2: Photon Map
		initializers::DescriptorSetLayoutBinding(2, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
		// Binding 3: Cloud Properties (read)
		initializers::DescriptorSetLayoutBinding(3, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 4: Parameters (read)
		initializers::DescriptorSetLayoutBinding(4, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
		// Binding 5: Photon Map Properties
		initializers::DescriptorSetLayoutBinding(5, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
	};
	m_descriptorSetLayout = new VulkanDescriptorSetLayout(m_device, peSetLayoutBindings);

	// Path tracer pipeline;
	std::vector<VkPushConstantRange> pePushConstantRanges
	{
		initializers::PushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants))
	};
	std::vector<VkDescriptorSetLayout> peSetLayouts
	{
		m_descriptorSetLayout->GetLayout()
	};

	m_pipelineLayout = new VulkanPipelineLayout(m_device, peSetLayouts, pePushConstantRanges);
	m_pipeline = new VulkanComputePipeline(m_device, m_pipelineLayout, m_shader);
}

RenderTechniquePPM::~RenderTechniquePPM()
{
	delete m_ptShader;
	delete m_ptDescriptorSetLayout;
	delete m_ptPipeline;
	delete m_ptPipelineLayout;

	delete m_shader;
	delete m_descriptorSetLayout;
	delete m_pipeline;
	delete m_pipelineLayout;

	ClearFrameResources();
	ClearPhotonMap();
}

void RenderTechniquePPM::ClearPhotonMap()
{
	if (m_photonMap)
	{
		delete m_collisionMapView;
		delete m_collisionMap;
		delete m_photonMapView;
		delete m_photonMap;

		m_photonMap = nullptr;
		m_photonMapView = nullptr;
		m_collisionMap = nullptr;
		m_collisionMapView = nullptr;
	}
}

void RenderTechniquePPM::AllocatePhotonMap(VulkanBuffer* photonMapPropertiesBuffer)
{
	m_photonMap = new VulkanImage(
		m_device,
		VK_FORMAT_R64G64B64A64_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
		m_photonMapProperties->cellCount.x, // Photon intensity, position, and incident direction - 18 bytes
		m_photonMapProperties->cellCount.y,
		m_photonMapProperties->cellCount.z);
	m_photonMapView = new VulkanImageView(m_device, m_photonMap);

	m_collisionMap = new VulkanImage(
		m_device,
		VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
		m_photonMapProperties->cellCount.x,
		m_photonMapProperties->cellCount.y,
		m_photonMapProperties->cellCount.z);
	m_collisionMapView = new VulkanImageView(m_device, m_collisionMap);


	std::vector<VkWriteDescriptorSet> writes;
	uint32_t setSize = static_cast<uint32_t>(m_descriptorSets.size() / 2);

	auto photonMapInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, m_photonMapView->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);
	auto collisionMapInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, m_collisionMapView->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);
	auto photonMapPropertiesInfo = initializers::DescriptorBufferInfo(photonMapPropertiesBuffer->GetBuffer(), 0, photonMapPropertiesBuffer->GetSize());
	for (size_t i = 0; i < setSize; i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &photonMapInfo));
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5, &collisionMapInfo));
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &photonMapPropertiesInfo));
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i + setSize], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, &photonMapInfo));
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i + setSize], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, &photonMapPropertiesInfo));
	};
	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void RenderTechniquePPM::SetFrameResources(std::vector<VulkanImage*>& frameImages, std::vector<VulkanImageView*>& frameImageViews, VulkanSwapchain* swapchain)
{
	m_images = frameImages;
	m_imageViews = frameImageViews;
	m_swapchain = swapchain;

	// Update compute bindings for output image
	std::vector<VkWriteDescriptorSet> writes;
	uint32_t setSize = static_cast<uint32_t>(m_descriptorSets.size() / 2);
	for (size_t i = setSize; i < m_descriptorSets.size(); i++)
	{
		auto imageInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, m_imageViews[i]->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &imageInfo));
	};
	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void RenderTechniquePPM::ClearFrameResources()
{
	m_imageViews.clear();
	m_images.clear();
	m_swapchain = nullptr;
}

uint32_t RenderTechniquePPM::GetRequiredSetCount() const
{
	return 2; // Photon Tracing and Estimate sets
}

void RenderTechniquePPM::GetDescriptorPoolSizes(std::vector<VkDescriptorPoolSize>& outPoolSizes) const
{
	outPoolSizes.push_back(initializers::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2));			// Photon and Collision maps
}

void RenderTechniquePPM::QueueUpdateCloudData(VkDescriptorBufferInfo& cloudBufferInfo, unsigned int frameNr)
{
	uint32_t setSize = static_cast<uint32_t>(m_descriptorSets.size() / 2);
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr + setSize], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo));
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &cloudBufferInfo));
}

void RenderTechniquePPM::QueueUpdateCloudDataSampler(VkDescriptorImageInfo& cloudImageInfo, unsigned int frameNr)
{
	uint32_t setSize = static_cast<uint32_t>(m_descriptorSets.size() / 2);
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &cloudImageInfo));
}

void RenderTechniquePPM::QueueUpdateCameraProperties(VkDescriptorBufferInfo& cameraBufferInfo, unsigned int frameNr)
{
	uint32_t setSize = static_cast<uint32_t>(m_descriptorSets.size() / 2);
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr + setSize], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &cameraBufferInfo));
}

void RenderTechniquePPM::QueueUpdateParameters(VkDescriptorBufferInfo& parametersBufferInfo, unsigned int frameNr)
{
	uint32_t setSize = static_cast<uint32_t>(m_descriptorSets.size() / 2);
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr + setSize], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &parametersBufferInfo));
	m_writeQueue.push_back(initializers::WriteDescriptorSet(m_descriptorSets[frameNr], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &parametersBufferInfo));
}

void RenderTechniquePPM::QueueUpdateShadowVolume(VkDescriptorBufferInfo& shadowVolumeBufferInfo, unsigned int frameNr)
{
}

void RenderTechniquePPM::QueueUpdateShadowVolumeSampler(VkDescriptorImageInfo& shadowVolumeImageInfo, unsigned int frameNr)
{
}

void RenderTechniquePPM::RecordDrawCommands(VkCommandBuffer commandBuffer, unsigned int currentFrame, unsigned int imageIndex)
{
	uint32_t setSize = static_cast<uint32_t>(m_descriptorSets.size() / 2);

	// Photon Tracing
	{
		// Change Photon Map Image Layout to writeable
		utilities::CmdTransitionImageLayout(commandBuffer, m_photonMap->GetImage(), m_photonMap->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		// Change Photon Collision Image Layout to writeable
		utilities::CmdTransitionImageLayout(commandBuffer, m_collisionMap->GetImage(), m_collisionMap->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		// Bind compute pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ptPipeline->GetPipeline());

		// Bind descriptor set (resources)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ptPipelineLayout->GetPipelineLayout(), 0, setSize, m_descriptorSets.data(), 0, nullptr);

		// Start compute shader
		vkCmdDispatch(commandBuffer, m_cameraProperties->GetWidth(), m_cameraProperties->GetHeight(), 1);
	}

	// Photon Estimate
	{
		// Change Result Image Layout to writeable
		utilities::CmdTransitionImageLayout(commandBuffer, m_images[currentFrame]->GetImage(), m_images[currentFrame]->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		// Bind compute pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline->GetPipeline());

		// Bind descriptor set (resources)
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout->GetPipelineLayout(), 0, setSize, m_descriptorSets.data() + setSize, 0, nullptr);

		// Start compute shader
		vkCmdDispatch(commandBuffer, m_cameraProperties->GetWidth(), m_cameraProperties->GetHeight(), 1);
	}

	// Change swapchain image layout to dst blit
	utilities::CmdTransitionImageLayout(commandBuffer, m_swapchain->GetSwapchainImages()[imageIndex], m_swapchain->GetImageFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// Change result image layout to scr blit
	utilities::CmdTransitionImageLayout(commandBuffer, m_images[currentFrame]->GetImage(), m_images[currentFrame]->GetFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

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
