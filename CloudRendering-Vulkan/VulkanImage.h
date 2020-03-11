#pragma once

class VulkanDevice;

class VulkanImage
{
public:
	VulkanImage(VulkanDevice* device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
	~VulkanImage();

	VkImage GetImage();
	VkFormat GetFormat();
	VkExtent3D GetExtents();

private:
	VulkanDevice* m_device = nullptr;
	VkImage m_image = VK_NULL_HANDLE;
	VkFormat m_format = VK_FORMAT_R8G8B8A8_SNORM;
	VkDeviceMemory m_deviceMemory;

	VkExtent3D m_extents;
};
