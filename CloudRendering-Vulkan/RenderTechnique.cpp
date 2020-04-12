#include "stdafx.h"
#include "RenderTechnique.h"

#include "VulkanPipelineLayout.h"

VkPipelineLayout RenderTechnique::GetPipelineLayout() const
{
	return m_pipelineLayout->GetPipelineLayout();
}

void RenderTechnique::GetDescriptorSetLayout(std::vector<VkDescriptorSetLayout>& outSetLayouts) const
{
	outSetLayouts.push_back(m_descriptorSetLayout->GetLayout());
}

void RenderTechnique::UpdateDescriptorSets()
{
	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(m_writeQueue.size()), m_writeQueue.data(), 0, nullptr);
	m_writeQueue.clear();
}

void RenderTechnique::CreateFrameResources()
{
	m_image = new VulkanImage(
		m_device,
		VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		static_cast<uint32_t>(m_cameraProperties->GetWidth()),
		static_cast<uint32_t>(m_cameraProperties->GetHeight()));
	m_imageView = new VulkanImageView(m_device, m_image);


	// Update compute bindings for output image
	std::vector<VkWriteDescriptorSet> writes;
	auto imageInfo = initializers::DescriptorImageInfo(VK_NULL_HANDLE, m_imageView->GetImageView(), VK_IMAGE_LAYOUT_GENERAL);
	for (size_t i = 0; i < m_descriptorSets.size(); i++)
	{
		writes.push_back(initializers::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, &imageInfo));
	};
	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void RenderTechnique::ClearFrameResources()
{
	if (m_imageView)
	{
		delete m_imageView;
	}
	if (m_image)
	{
		delete m_image;
	}

	m_imageView = nullptr;
	m_image = nullptr;
}
