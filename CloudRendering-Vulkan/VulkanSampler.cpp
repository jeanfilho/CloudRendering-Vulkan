#include "stdafx.h"
#include "VulkanSampler.h"

#include "VulkanDevice.h"

VulkanSampler::VulkanSampler(VulkanDevice* device)
{
	m_device = device;

	vkCreateSampler(m_device->GetDevice(), &initializers::SamplerCreateInfo(), nullptr, &m_sampler);
}

VulkanSampler::~VulkanSampler()
{
	if (m_sampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(m_device->GetDevice(), m_sampler, nullptr);
	}
}

VkSampler VulkanSampler::GetSampler()
{
	return m_sampler;
}
