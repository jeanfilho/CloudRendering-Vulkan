#include "stdafx.h"
#include "VulkanCommandPool.h"

#include "VulkanDevice.h"

VulkanCommandPool::VulkanCommandPool(VulkanDevice* device, uint32_t queueFamilyIndex)
{
	m_device = device;

	VkCommandPoolCreateInfo commandPoolInfo = initializers::CommandPoolCreateInfo(queueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

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
	vkResetCommandPool(m_device->GetDevice(), m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

	m_commandBuffers.resize(size);

	VkCommandBufferAllocateInfo allocInfo = initializers::CommandBufferAllocateInfo(m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()));
	ValidCheck(vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, m_commandBuffers.data()));

	return m_commandBuffers;
}

void VulkanCommandPool::ClearCommandBuffers()
{
	vkFreeCommandBuffers(m_device->GetDevice(), m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
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
