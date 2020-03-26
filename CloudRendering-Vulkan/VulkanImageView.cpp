#include "stdafx.h"
#include "VulkanImageView.h"

#include "VulkanDevice.h"
#include "VulkanImage.h"

VulkanImageView::VulkanImageView(VulkanDevice* device, VulkanImage* image)
{
	m_device = device;

	VkExtent3D extents = image->GetExtent();
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_1D;
	if (extents.depth > 1)
	{
		viewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else if (extents.height > 1)
	{
		viewType = VK_IMAGE_VIEW_TYPE_2D;
	}

	VkImageViewCreateInfo viewInfo = initializers::ImageViewCreateInfo(image->GetImage(), viewType, image->GetFormat());
	ValidCheck(vkCreateImageView(m_device->GetDevice(), &viewInfo, nullptr, &m_imageView));
}

VulkanImageView::VulkanImageView(VulkanDevice* device, VkImage image, VkFormat format, VkImageViewType viewType)
{
	m_device = device;

	VkImageViewCreateInfo viewInfo = initializers::ImageViewCreateInfo(image, viewType, format);
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
