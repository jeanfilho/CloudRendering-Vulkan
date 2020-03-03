#include "stdafx.h"
#include "VulkanBuffer.h"

#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"

VulkanBuffer::VulkanBuffer(VulkanDevice* device, void* ptr, unsigned int elementSize, unsigned int count /*= 1*/)
{
	m_device = device;
	m_ptr = ptr;
	m_elementSize = elementSize;
	m_count = count;
	m_totalSize = (VkDeviceSize)m_elementSize * m_count;
	CreateBuffer();
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

void VulkanBuffer::CreateBuffer()
{
	VkBufferCreateInfo bufferInfo = initializers::BufferCreateInfo(m_totalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	ValidCheck(vkCreateBuffer(m_device->GetDevice(), &bufferInfo, nullptr, &m_buffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device->GetDevice(), m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = initializers::MemoryAllocateInfo(memRequirements.size, FindMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memRequirements.memoryTypeBits));

	ValidCheck(vkAllocateMemory(m_device->GetDevice(), &allocInfo, nullptr, &m_deviceMemory));
	ValidCheck(vkBindBufferMemory(m_device->GetDevice(), m_buffer, m_deviceMemory, 0));
	ValidCheck(vkMapMemory(m_device->GetDevice(), m_deviceMemory, 0, memRequirements.size, 0, &m_mappedMemory));
}

uint32_t VulkanBuffer::FindMemoryType(VkMemoryPropertyFlags props, uint32_t typeFilter)
{
	for (uint32_t i = 0; i < m_device->GetPhysicalDevice()->GetPhysicalDeviceMemoryProperties().memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (m_device->GetPhysicalDevice()->GetPhysicalDeviceMemoryProperties().memoryTypes[i].propertyFlags & props) == props)
		{
			return i;
		}
	}

	assert(0 && "No available memory properties");
	return -1;
}
