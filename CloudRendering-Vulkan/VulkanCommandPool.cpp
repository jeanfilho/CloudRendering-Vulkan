#include "stdafx.h"
#include "VulkanCommandPool.h"

#include "VulkanDevice.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice* device, uint32_t queueFamilyIndex)
{
	m_device = device;

	VkCommandPoolCreateInfo commandPoolInfo = initializers::CommandPoolCreateInfo(queueFamilyIndex);

	ValidCheck(vkCreateCommandPool(m_device->GetDevice(), &commandPoolInfo, nullptr, &m_commandPool));
}

VulkanCommandPool::~VulkanCommandPool()
{
	if (m_commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(m_device->GetDevice(), m_commandPool, nullptr);
	}
}

std::vector<VkCommandBuffer>& VulkanCommandPool::AllocateCommandBuffers(size_t size)
{
	m_commandBuffers.resize(size);

	VkCommandBufferAllocateInfo allocInfo = initializers::CommandBufferAllocateInfo(m_commandPool, m_commandBuffers.size());
	ValidCheck(vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, m_commandBuffers.data()));

	return m_commandBuffers;
}

void VulkanCommandPool::ClearCommandBuffers()
{
	vkFreeCommandBuffers(m_device->GetDevice(), m_commandPool, m_commandBuffers.size(), m_commandBuffers.data());
	m_commandBuffers.clear();
}

VkCommandPool VulkanCommandPool::GetCommandPool()
{
	return m_commandPool;
}

std::vector<VkCommandBuffer>& VulkanCommandPool::GetCommandBuffers()
{
	return m_commandBuffers;
}
