#include "stdafx.h"
#include "RenderTechnique.h"

#include "VulkanPipelineLayout.h"

RenderTechnique::RenderTechnique(VulkanDevice* device, PushConstants* pushConstants) : m_device(device), m_pushConstants(pushConstants)
{
}

RenderTechnique::~RenderTechnique()
{
}

void RenderTechnique::UpdateDescriptorSets()
{
	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(m_writeQueue.size()), m_writeQueue.data(), 0, nullptr);
	m_writeQueue.clear();
}
