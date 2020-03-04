#include "stdafx.h"
#include "VulkanFence.h"

#include "VulkanDevice.h"

VulkanFence::VulkanFence(VulkanDevice* device)
{
	m_device = device;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	ValidCheck(vkCreateFence(m_device->GetDevice(), &fenceInfo, nullptr, &m_fence));
}

VulkanFence::~VulkanFence()
{
	if (m_fence != VK_NULL_HANDLE)
	{
		vkDestroyFence(m_device->GetDevice(), m_fence, nullptr);
	}
}

VkFence& VulkanFence::GetFence()
{
	return m_fence;
}
