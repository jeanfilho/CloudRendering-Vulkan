#include "stdafx.h"
#include "RenderTechnique.h"

#include "VulkanPipelineLayout.h"

VkPipelineLayout RenderTechnique::GetPipelineLayout() const
{
	return m_pipelineLayout->GetPipelineLayout();
}

void RenderTechnique::UpdateDescriptorSets()
{
	vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(m_writeQueue.size()), m_writeQueue.data(), 0, nullptr);
	m_writeQueue.clear();
}
