#include "stdafx.h"
#include "VulkanImage.h"

#include "VulkanDevice.h"

VulkanImage::VulkanImage(VulkanDevice* device, VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height /*= 1*/, uint32_t depth /*= 1*/)
{
	m_device = device;
	m_format = format;
	m_extents = { width, height, depth };

	VkImageType type = VK_IMAGE_TYPE_1D;
	if (depth > 1)
	{
		type = VK_IMAGE_TYPE_3D;
	}
	else if(height > 1)
	{
		type = VK_IMAGE_TYPE_2D;
	}

	VkImageCreateInfo imageInfo = initializers::ImageCreateInfo(type, m_format, m_extents, 1, 1, VK_SAMPLE_COUNT_1_BIT, usage);

	ValidCheck(vkCreateImage(m_device->GetDevice(), &imageInfo, nullptr, &m_image));

	// Get device properties for the requested texture format
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice()->GetPhysicaDevice(), format, &formatProperties);

	// Check if requested image format supports image storage operations
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

	// Allocate memory for image and bind
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device->GetDevice(), m_image, &memRequirements);

	VkMemoryAllocateInfo allocationInfo = initializers::MemoryAllocateInfo(memRequirements.size, m_device->FindMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memRequirements.memoryTypeBits));

	ValidCheck(vkAllocateMemory(m_device->GetDevice(), &allocationInfo, nullptr, &m_deviceMemory));
	ValidCheck(vkBindImageMemory(m_device->GetDevice(), m_image, m_deviceMemory, 0));
}

VulkanImage::~VulkanImage()
{
	if (m_image != VK_NULL_HANDLE)
	{
		vkDestroyImage(m_device->GetDevice(), m_image, nullptr);
		vkFreeMemory(m_device->GetDevice(), m_deviceMemory, nullptr);
	}
}

VkImage VulkanImage::GetImage()
{
	return m_image;
}

VkFormat VulkanImage::GetFormat()
{
	return m_format;
}

VkExtent3D VulkanImage::GetExtent()
{
	return m_extents;
}
