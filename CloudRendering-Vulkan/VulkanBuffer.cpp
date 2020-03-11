#include "stdafx.h"
#include "VulkanBuffer.h"

#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"

VulkanBuffer::VulkanBuffer(VulkanDevice* device, void* data, size_t elementSize, VkBufferUsageFlagBits usageFlags, size_t count /*= 1*/)
{
	m_device = device;
	m_ptr = data;
	m_elementSize = elementSize;
	m_count = count;
	m_totalSize = (VkDeviceSize)m_elementSize * m_count;
	AllocateBuffer(usageFlags);
}

VulkanBuffer::~VulkanBuffer()
{
	if (m_deviceMemory != VK_NULL_HANDLE)
	{
		vkUnmapMemory(m_device->GetDevice(), m_deviceMemory);
		vkFreeMemory(m_device->GetDevice(), m_deviceMemory, nullptr);
	}
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device->GetDevice(), m_buffer, nullptr);
	}
}

void VulkanBuffer::SetData()
{
	memcpy(m_mappedMemory, m_ptr, (size_t)m_totalSize);
}

void VulkanBuffer::SetData(size_t count)
{
	memcpy(m_mappedMemory, m_ptr, m_elementSize * count);
}

void VulkanBuffer::SetData(size_t startIndex, size_t count)
{
	memcpy(((char*)m_mappedMemory) + (startIndex * m_elementSize), ((char*)m_ptr) + (startIndex * m_elementSize), (size_t)m_elementSize * count);
}

VkBuffer VulkanBuffer::GetBuffer()
{
	return m_buffer;
}

VkDeviceSize VulkanBuffer::GetSize()
{
	return m_totalSize;
}

void VulkanBuffer::AllocateBuffer(VkBufferUsageFlagBits usageFlags)
{
	VkBufferCreateInfo bufferInfo = initializers::BufferCreateInfo(m_totalSize, usageFlags);

	ValidCheck(vkCreateBuffer(m_device->GetDevice(), &bufferInfo, nullptr, &m_buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device->GetDevice(), m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = initializers::MemoryAllocateInfo(memRequirements.size, m_device->FindMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memRequirements.memoryTypeBits));

	ValidCheck(vkAllocateMemory(m_device->GetDevice(), &allocInfo, nullptr, &m_deviceMemory));
	ValidCheck(vkBindBufferMemory(m_device->GetDevice(), m_buffer, m_deviceMemory, 0));
	ValidCheck(vkMapMemory(m_device->GetDevice(), m_deviceMemory, 0, memRequirements.size, 0, &m_mappedMemory));
}
