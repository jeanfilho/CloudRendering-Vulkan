#include "stdafx.h"
#include "VulkanBufferView.h"

#include "VulkanDevice.h"
#include "VulkanBuffer.h"

VulkanBufferView::VulkanBufferView(VulkanDevice* device, VulkanBuffer* buffer, VkFormat format)
{
	m_device = device;

	VkBufferViewCreateInfo viewInfo = initializers::BufferViewCreateInfo(buffer->GetBuffer(), format, 0, buffer->GetSize(), 0);

	ValidCheck(vkCreateBufferView(m_device->GetDevice(), &viewInfo, nullptr, &m_view));
}

VulkanBufferView::~VulkanBufferView()
{
	if (m_view != VK_NULL_HANDLE)
	{
		vkDestroyBufferView(m_device->GetDevice(), m_view, nullptr);
	}
}

VkBufferView& VulkanBufferView::GetBufferView()
{
	return m_view;
}
