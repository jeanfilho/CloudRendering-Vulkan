#pragma once

class VulkanDevice;

class VulkanImage
{
public:
	VulkanImage(VulkanDevice* device, VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height = 1, uint32_t depth = 1);
	~VulkanImage();

	VkImage GetImage();
	VkFormat GetFormat();
	VkExtent3D GetExtent();

private:
	VulkanDevice* m_device = nullptr;
	VkImage m_image = VK_NULL_HANDLE;
	VkFormat m_format = VK_FORMAT_R8G8B8A8_SNORM;
	VkDeviceMemory m_deviceMemory;

	VkExtent3D m_extents;
};
