#include "stdafx.h"
#include "VulkanImageView.h"

#include "VulkanDevice.h"

VulkanImageView::VulkanImageView(VulkanDevice* device, VkImage image, VkFormat format)
{
	m_device = device;

	VkImageViewCreateInfo viewInfo = initializers::ImageViewCreateInfo(image, format);
	ValidCheck(vkCreateImageView(m_device->GetDevice(), &viewInfo, nullptr, &m_imageView));
}

VulkanImageView::~VulkanImageView()
{
	if (m_imageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(m_device->GetDevice(), m_imageView, nullptr);
	}
}

VkImageView VulkanImageView::GetImageView()
{
	return m_imageView;
}
